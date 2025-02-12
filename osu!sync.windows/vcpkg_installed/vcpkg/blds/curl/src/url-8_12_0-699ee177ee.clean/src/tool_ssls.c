/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/
#include "tool_setup.h"

#include "curlx.h"
#include "tool_cfgable.h"
#include "tool_cb_dbg.h"
#include "tool_msgs.h"
#include "tool_setopt.h"
#include "tool_ssls.h"
#include "dynbuf.h"
#include "curl_base64.h"
#include "curl_get_line.h"

/* The maximum line length for an ecoded session ticket */
#define MAX_SSLS_LINE (64 * 1024)


static CURLcode tool_ssls_easy(struct GlobalConfig *global,
                               struct OperationConfig *config,
                               CURLSH *share, CURL **peasy)
{
  CURLcode result = CURLE_OK;

  *peasy = curl_easy_init();
  if(!*peasy)
    return CURLE_OUT_OF_MEMORY;

  result = curl_easy_setopt(*peasy, CURLOPT_SHARE, share);
  if(!result && (global->tracetype != TRACE_NONE)) {
    my_setopt(*peasy, CURLOPT_DEBUGFUNCTION, tool_debug_cb);
    my_setopt(*peasy, CURLOPT_DEBUGDATA, config);
    my_setopt(*peasy, CURLOPT_VERBOSE, 1L);
  }
  return result;
}

CURLcode tool_ssls_load(struct GlobalConfig *global,
                        struct OperationConfig *config,
                        CURLSH *share, const char *filename)
{
  FILE *fp;
  CURL *easy = NULL;
  struct dynbuf buf;
  unsigned char *shmac = NULL, *sdata = NULL;
  char *c, *line, *end;
  size_t shmac_len, sdata_len;
  CURLcode r = CURLE_OK;
  int i, imported;

  curlx_dyn_init(&buf, MAX_SSLS_LINE);
  fp = fopen(filename, FOPEN_READTEXT);
  if(!fp) { /* ok if it does not exist */
    notef(global, "SSL session file does not exist (yet?): %s", filename);
    goto out;
  }

  r = tool_ssls_easy(global, config, share, &easy);
  if(r)
    goto out;

  i = imported = 0;
  while(Curl_get_line(&buf, fp)) {
    ++i;
    curl_free(shmac);
    curl_free(sdata);
    line = Curl_dyn_ptr(&buf);
    while(*line && ISBLANK(*line))
      line++;
    if(*line == '#')
      /* skip commented lines */
      continue;

    c = memchr(line, ':', strlen(line));
    if(!c) {
      warnf(global, "unrecognized line %d in ssl session file %s",
            i, filename);
      continue;
    }
    *c = '\0';
    r = curlx_base64_decode(line, &shmac, &shmac_len);
    if(r) {
      warnf(global, "invalid shmax base64 encoding in line %d", i);
      continue;
    }
    line = c + 1;
    end = line + strlen(line) - 1;
    while((end > line) && (*end == '\n' || *end == '\r' || ISBLANK(*line))) {
      *end = '\0';
      --end;
    }
    r = curlx_base64_decode(line, &sdata, &sdata_len);
    if(r) {
      warnf(global, "invalid sdata base64 encoding in line %d: %s", i, line);
      continue;
    }

    r = curl_easy_ssls_import(easy, NULL, shmac, shmac_len, sdata, sdata_len);
    if(r) {
      warnf(global, "import of session from line %d rejected(%d)", i, r);
      continue;
    }
    ++imported;
  }
  r = CURLE_OK;

out:
  if(easy)
    curl_easy_cleanup(easy);
  if(fp)
    fclose(fp);
  curlx_dyn_free(&buf);
  curl_free(shmac);
  curl_free(sdata);
  return r;
}

struct tool_ssls_ctx {
  struct GlobalConfig *global;
  FILE *fp;
  int exported;
};

static CURLcode tool_ssls_exp(CURL *easy, void *userptr,
                              const char *session_key,
                              const unsigned char *shmac, size_t shmac_len,
                              const unsigned char *sdata, size_t sdata_len,
                              curl_off_t valid_until, int ietf_tls_id,
                              const char *alpn, size_t earlydata_max)
{
  struct tool_ssls_ctx *ctx = userptr;
  char *enc = NULL;
  size_t enc_len;
  CURLcode r;

  (void)easy;
  (void)valid_until;
  (void)ietf_tls_id;
  (void)alpn;
  (void)earlydata_max;
  if(!ctx->exported)
    fputs("# Your SSL session cache. https://curl.se/docs/ssl-sessions.html\n"
        "# This file was generated by libcurl! Edit at your own risk.\n",
        ctx->fp);

  r = curlx_base64_encode((const char *)shmac, shmac_len, &enc, &enc_len);
  if(r)
    goto out;
  r = CURLE_WRITE_ERROR;
  if(enc_len != fwrite(enc, 1, enc_len, ctx->fp))
    goto out;
  if(EOF == fputc(':', ctx->fp))
    goto out;
  curl_free(enc);
  r = curlx_base64_encode((const char *)sdata, sdata_len, &enc, &enc_len);
  if(r)
    goto out;
  r = CURLE_WRITE_ERROR;
  if(enc_len != fwrite(enc, 1, enc_len, ctx->fp))
    goto out;
  if(EOF == fputc('\n', ctx->fp))
    goto out;
  r = CURLE_OK;
  ctx->exported++;
out:
  if(r)
    warnf(ctx->global, "Warning: error saving SSL session for '%s': %d",
          session_key, r);
  curl_free(enc);
  return r;
}

CURLcode tool_ssls_save(struct GlobalConfig *global,
                        struct OperationConfig *config,
                        CURLSH *share, const char *filename)
{
  struct tool_ssls_ctx ctx;
  CURL *easy = NULL;
  CURLcode r = CURLE_OK;

  ctx.global = global;
  ctx.exported = 0;
  ctx.fp = fopen(filename, FOPEN_WRITETEXT);
  if(!ctx.fp) {
    warnf(global, "Warning: Failed to create SSL session file %s", filename);
    goto out;
  }

  r = tool_ssls_easy(global, config, share, &easy);
  if(r)
    goto out;

  r = curl_easy_ssls_export(easy, tool_ssls_exp, &ctx);

out:
  if(easy)
    curl_easy_cleanup(easy);
  if(ctx.fp)
    fclose(ctx.fp);
  return r;
}
