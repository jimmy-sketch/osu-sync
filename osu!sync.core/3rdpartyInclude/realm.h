/*************************************************************************
 *
 * Copyright 2024 Realm Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **************************************************************************/

#ifndef REALM_H
#define REALM_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <realm/error_codes.h>

#if defined(_WIN32) || defined(__CYGWIN__)

#if defined(Realm_EXPORTS)
// Exporting Win32 symbols
#define RLM_EXPORT __declspec(dllexport)
#else
// Importing Win32 symbols. Note: Clients linking statically should define
// RLM_NO_DLLIMPORT.
#if !defined(RLM_NO_DLLIMPORT)
#define RLM_EXPORT __declspec(dllimport)
#else
#define RLM_EXPORT
#endif // RLM_NO_DLLIMPORT
#endif // Realm_EXPORTS

#else
// Not Win32
#define RLM_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
#define RLM_API extern "C" RLM_EXPORT
#define RLM_API_NOEXCEPT noexcept
#else
#define RLM_API RLM_EXPORT
#define RLM_API_NOEXCEPT
#endif // __cplusplus

// Some platforms don't support anonymous unions in structs.
// RLM_NO_ANON_UNIONS allows definining a member name for unions in structs where
// RLM_ANON_UNION_MEMBER(name) is used.
#ifdef RLM_NO_ANON_UNIONS
#define RLM_ANON_UNION_MEMBER(name) name
#else
#define RLM_ANON_UNION_MEMBER(name)
#endif

// Some platforms can benefit from redefining the userdata type to another type known to the tooling.
// For example, Dart with its ffigen utility can generate cleaner code if we define realm_userdata_t as Dart_Handle,
// which is a pointer to an opaque struct treated specially by the Dart code generator.
// WARNING: only define this to a pointer type, anything else breaks the ABI.
#ifndef realm_userdata_t
#define realm_userdata_t void*
#endif

typedef struct shared_realm realm_t;
typedef struct realm_schema realm_schema_t;
typedef struct realm_scheduler realm_scheduler_t;
typedef struct realm_work_queue realm_work_queue_t;
typedef struct realm_thread_safe_reference realm_thread_safe_reference_t;
typedef void (*realm_free_userdata_func_t)(realm_userdata_t userdata);
typedef realm_userdata_t (*realm_clone_userdata_func_t)(const realm_userdata_t userdata);
typedef void (*realm_on_object_store_thread_callback_t)(realm_userdata_t userdata);
typedef bool (*realm_on_object_store_error_callback_t)(realm_userdata_t userdata, const char*);
typedef struct realm_key_path_array realm_key_path_array_t;

/* Accessor types */
typedef struct realm_object realm_object_t;

typedef struct realm_list realm_list_t;
typedef struct realm_set realm_set_t;
typedef struct realm_dictionary realm_dictionary_t;

/* Query types */
typedef struct realm_query realm_query_t;
typedef struct realm_results realm_results_t;

/* Config types */
typedef struct realm_config realm_config_t;
typedef struct realm_app_config realm_app_config_t;
typedef struct realm_sync_client_config realm_sync_client_config_t;
typedef struct realm_sync_config realm_sync_config_t;
typedef bool (*realm_migration_func_t)(realm_userdata_t userdata, realm_t* old_realm, realm_t* new_realm,
                                       const realm_schema_t* schema);
typedef bool (*realm_data_initialization_func_t)(realm_userdata_t userdata, realm_t* realm);
typedef bool (*realm_should_compact_on_launch_func_t)(realm_userdata_t userdata, uint64_t total_bytes,
                                                      uint64_t used_bytes);

typedef enum realm_schema_mode {
    RLM_SCHEMA_MODE_AUTOMATIC,
    RLM_SCHEMA_MODE_IMMUTABLE,
    RLM_SCHEMA_MODE_READ_ONLY,
    RLM_SCHEMA_MODE_SOFT_RESET_FILE,
    RLM_SCHEMA_MODE_HARD_RESET_FILE,
    RLM_SCHEMA_MODE_ADDITIVE_DISCOVERED,
    RLM_SCHEMA_MODE_ADDITIVE_EXPLICIT,
    RLM_SCHEMA_MODE_MANUAL,
} realm_schema_mode_e;

typedef enum realm_schema_subset_mode {
    RLM_SCHEMA_SUBSET_MODE_STRICT,
    RLM_SCHEMA_SUBSET_MODE_ALL_CLASSES,
    RLM_SCHEMA_SUBSET_MODE_ALL_PROPERTIES,
    RLM_SCHEMA_SUBSET_MODE_COMPLETE
} realm_schema_subset_mode_e;

/* Key types */
typedef uint32_t realm_class_key_t;
typedef int64_t realm_property_key_t;
typedef int64_t realm_object_key_t;
typedef uint64_t realm_version_t;

static const realm_class_key_t RLM_INVALID_CLASS_KEY = ((uint32_t)-1) >> 1;
static const realm_property_key_t RLM_INVALID_PROPERTY_KEY = -1;
static const realm_object_key_t RLM_INVALID_OBJECT_KEY = -1;

/* Value types */

typedef enum realm_value_type {
    RLM_TYPE_NULL,
    RLM_TYPE_INT,
    RLM_TYPE_BOOL,
    RLM_TYPE_STRING,
    RLM_TYPE_BINARY,
    RLM_TYPE_TIMESTAMP,
    RLM_TYPE_FLOAT,
    RLM_TYPE_DOUBLE,
    RLM_TYPE_DECIMAL128,
    RLM_TYPE_OBJECT_ID,
    RLM_TYPE_LINK,
    RLM_TYPE_UUID,
    RLM_TYPE_LIST,
    RLM_TYPE_DICTIONARY,
} realm_value_type_e;

typedef enum realm_schema_validation_mode {
    RLM_SCHEMA_VALIDATION_BASIC = 0,
    RLM_SCHEMA_VALIDATION_SYNC_PBS = 1,
    RLM_SCHEMA_VALIDATION_REJECT_EMBEDDED_ORPHANS = 2,
    RLM_SCHEMA_VALIDATION_SYNC_FLX = 4
} realm_schema_validation_mode_e;

/**
 * Represents a view over a UTF-8 string buffer. The buffer is unowned by this struct.
 *
 * This string can have three states:
 * - null
 *   When the data member is NULL.
 * - empty
 *   When the data member is non-NULL, and the size member is 0. The actual contents of the data member in this case
 * don't matter.
 * - non-empty
 *   When the data member is non-NULL, and the size member is greater than 0.
 *
 */
typedef struct realm_string {
    const char* data;
    size_t size;
} realm_string_t;

typedef struct realm_binary {
    const uint8_t* data;
    size_t size;
} realm_binary_t;

typedef struct realm_timestamp {
    int64_t seconds;
    int32_t nanoseconds;
} realm_timestamp_t;

typedef struct realm_decimal128 {
    uint64_t w[2];
} realm_decimal128_t;

typedef struct realm_link {
    realm_class_key_t target_table;
    realm_object_key_t target;
} realm_link_t;

typedef struct realm_object_id {
    uint8_t bytes[12];
} realm_object_id_t;

typedef struct realm_uuid {
    uint8_t bytes[16];
} realm_uuid_t;

typedef struct realm_value {
    union {
        int64_t integer;
        bool boolean;
        realm_string_t string;
        realm_binary_t binary;
        realm_timestamp_t timestamp;
        float fnum;
        double dnum;
        realm_decimal128_t decimal128;
        realm_object_id_t object_id;
        realm_uuid_t uuid;
        realm_link_t link;

        char data[16];
    } RLM_ANON_UNION_MEMBER(values);
    realm_value_type_e type;
} realm_value_t;
typedef struct realm_query_arg {
    size_t nb_args;
    bool is_list;
    realm_value_t* arg;
} realm_query_arg_t;

typedef struct realm_version_id {
    uint64_t version;
    uint64_t index;
} realm_version_id_t;


/* Error types */
typedef struct realm_async_error realm_async_error_t;
typedef unsigned realm_error_categories;

typedef struct realm_error {
    realm_errno_e error;
    realm_error_categories categories;
    const char* message;
    // When error is RLM_ERR_CALLBACK this is an opaque pointer to an SDK-owned error object
    // thrown by user code inside a callback with realm_register_user_code_callback_error(), otherwise null.
    void* user_code_error;
    const char* path;
} realm_error_t;

/* Schema types */

typedef enum realm_column_attr {
    // Values matching `realm::ColumnAttr`.
    RLM_COLUMN_ATTR_NONE = 0,
    RLM_COLUMN_ATTR_INDEXED = 1,
    RLM_COLUMN_ATTR_UNIQUE = 2,
    RLM_COLUMN_ATTR_RESERVED = 4,
    RLM_COLUMN_ATTR_STRONG_LINKS = 8,
    RLM_COLUMN_ATTR_NULLABLE = 16,
    RLM_COLUMN_ATTR_LIST = 32,
    RLM_COLUMN_ATTR_DICTIONARY = 64,
    RLM_COLUMN_ATTR_COLLECTION = 64 + 32,
} realm_column_attr_e;

typedef enum realm_property_type {
    // Values matching `realm::ColumnType`.
    RLM_PROPERTY_TYPE_INT = 0,
    RLM_PROPERTY_TYPE_BOOL = 1,
    RLM_PROPERTY_TYPE_STRING = 2,
    RLM_PROPERTY_TYPE_BINARY = 4,
    RLM_PROPERTY_TYPE_MIXED = 6,
    RLM_PROPERTY_TYPE_TIMESTAMP = 8,
    RLM_PROPERTY_TYPE_FLOAT = 9,
    RLM_PROPERTY_TYPE_DOUBLE = 10,
    RLM_PROPERTY_TYPE_DECIMAL128 = 11,
    RLM_PROPERTY_TYPE_OBJECT = 12,
    RLM_PROPERTY_TYPE_LINKING_OBJECTS = 14,
    RLM_PROPERTY_TYPE_OBJECT_ID = 15,
    RLM_PROPERTY_TYPE_UUID = 17,
} realm_property_type_e;

typedef enum realm_collection_type {
    RLM_COLLECTION_TYPE_NONE = 0,
    RLM_COLLECTION_TYPE_LIST = 1,
    RLM_COLLECTION_TYPE_SET = 2,
    RLM_COLLECTION_TYPE_DICTIONARY = 4,
} realm_collection_type_e;

typedef struct realm_property_info {
    const char* name;
    const char* public_name;
    realm_property_type_e type;
    realm_collection_type_e collection_type;

    const char* link_target;
    const char* link_origin_property_name;
    realm_property_key_t key;
    int flags;
} realm_property_info_t;

typedef struct realm_class_info {
    const char* name;
    const char* primary_key;
    size_t num_properties;
    size_t num_computed_properties;
    realm_class_key_t key;
    int flags;
} realm_class_info_t;

typedef enum realm_class_flags {
    RLM_CLASS_NORMAL = 0,
    RLM_CLASS_EMBEDDED = 1,
    RLM_CLASS_ASYMMETRIC = 2,
    RLM_CLASS_MASK = 3,
} realm_class_flags_e;

typedef enum realm_property_flags {
    RLM_PROPERTY_NORMAL = 0,
    RLM_PROPERTY_NULLABLE = 1,
    RLM_PROPERTY_PRIMARY_KEY = 2,
    RLM_PROPERTY_INDEXED = 4,
    RLM_PROPERTY_FULLTEXT_INDEXED = 8,
} realm_property_flags_e;


/* Notification types */
typedef struct realm_notification_token realm_notification_token_t;
typedef struct realm_callback_token realm_callback_token_t;
typedef struct realm_refresh_callback_token realm_refresh_callback_token_t;
typedef struct realm_object_changes realm_object_changes_t;
typedef struct realm_collection_changes realm_collection_changes_t;
typedef struct realm_dictionary_changes realm_dictionary_changes_t;
typedef void (*realm_on_object_change_func_t)(realm_userdata_t userdata, const realm_object_changes_t*);
typedef void (*realm_on_collection_change_func_t)(realm_userdata_t userdata, const realm_collection_changes_t*);
typedef void (*realm_on_dictionary_change_func_t)(realm_userdata_t userdata, const realm_dictionary_changes_t*);
typedef void (*realm_on_realm_change_func_t)(realm_userdata_t userdata);
typedef void (*realm_on_realm_refresh_func_t)(realm_userdata_t userdata);
typedef void (*realm_async_begin_write_func_t)(realm_userdata_t userdata);
typedef void (*realm_async_commit_func_t)(realm_userdata_t userdata, bool error, const char* desc);

/**
 * Callback for realm schema changed notifications.
 *
 * @param new_schema The new schema. This object is released after the callback returns.
 *                   Preserve it with realm_clone() if you wish to keep it around for longer.
 */
typedef void (*realm_on_schema_change_func_t)(realm_userdata_t userdata, const realm_schema_t* new_schema);

/* Scheduler types */
typedef void (*realm_scheduler_notify_func_t)(realm_userdata_t userdata, realm_work_queue_t* work_queue);
typedef bool (*realm_scheduler_is_on_thread_func_t)(realm_userdata_t userdata);
typedef bool (*realm_scheduler_is_same_as_func_t)(const realm_userdata_t scheduler_userdata_1,
                                                  const realm_userdata_t scheduler_userdata_2);
typedef bool (*realm_scheduler_can_deliver_notifications_func_t)(realm_userdata_t userdata);
typedef realm_scheduler_t* (*realm_scheduler_default_factory_func_t)(realm_userdata_t userdata);

/* Sync Socket Provider types */
typedef struct realm_websocket_endpoint {
    const char* address;    // Host address
    uint16_t port;          // Host port number
    const char* path;       // Includes access token in query.
    const char** protocols; // Array of one or more websocket protocols
    size_t num_protocols;   // Number of protocols in array
    bool is_ssl;            // true if SSL should be used
} realm_websocket_endpoint_t;

// The following definitions are intended for internal state and structures
// used by the Sync Client. These values should be retained by the Platform
// Networking CAPI implementation so they can be provided back to the Platform
// Networking CAPI functions.
typedef struct realm_sync_socket realm_sync_socket_t;
typedef struct realm_sync_socket_callback realm_sync_socket_post_callback_t;
typedef struct realm_sync_socket_callback realm_sync_socket_timer_callback_t;
typedef struct realm_sync_socket_callback realm_sync_socket_write_callback_t;
typedef void* realm_sync_socket_timer_t;
typedef void* realm_sync_socket_websocket_t;
typedef struct realm_websocket_observer realm_websocket_observer_t;

// Called when the Sync Client posts a callback handler to be run within the context
// of the event loop.
// The post_callback pointer does not need to be released by the CAPI implementation.
typedef void (*realm_sync_socket_post_func_t)(realm_userdata_t userdata,
                                              realm_sync_socket_post_callback_t* post_callback);

// Called when a Sync Socket Timer is being created, which will start the timer countdown
// immediately. The Timer CAPI implementation will need to be stored locally so it can
// be used when calling realm_sync_socket_timer_complete() when the timer countdown
// reaches 0 (i.e. expired) or realm_sync_socket_timer_canceled() when the timer is canceled.
// The timer_callback pointer does not need to be released by the CAPI implementation.
typedef realm_sync_socket_timer_t (*realm_sync_socket_create_timer_func_t)(
    realm_userdata_t userdata, uint64_t delay_ms, realm_sync_socket_timer_callback_t* timer_callback);

// Called when a Sync Socket Timer has been explicitly canceled or the Timer is being
// destroyed. Use the realm_sync_socket_timer_canceled() function to notify the Sync Client
// that the timer cancel is complete. NOTE: This function will always be called before the
// timer is destroyed (even if the timer has completed), but the timer callback should only
// be executed one time.
typedef void (*realm_sync_socket_timer_canceled_func_t)(realm_userdata_t userdata,
                                                        realm_sync_socket_timer_t timer_userdata);

// Called when the timer object has been destroyed so the Sync Socket Timer CAPI
// implementation can clean up its timer resources.
typedef void (*realm_sync_socket_timer_free_func_t)(realm_userdata_t userdata,
                                                    realm_sync_socket_timer_t timer_userdata);

// Called when the Sync Client is initiating a connection to the server. The endpoint
// structure contains the server address/URL and the websocket_observer will need to
// be stored locally in the WebSocket CAPI implementation so it can be used with the
// realm_sync_socket_websocket_[connected|message|error|closed]() functions when
// providing WebSocket status or data to the Sync Client.
typedef realm_sync_socket_websocket_t (*realm_sync_socket_connect_func_t)(
    realm_userdata_t userdata, realm_websocket_endpoint_t endpoint, realm_websocket_observer_t* websocket_observer);

// Called by a connection in the Sync Client when it needs to send data to the server. The
// write_callback is used with realm_sync_socket_write_complete() to inform the connection
// that the data has been transferred successfully.
// If an error occurs during the async write operation, it needs to be provided to the
// write_callback handler, and the websocket is exepected to be closed by calling
// realm_sync_socket_websocket_error() followed by providing the error code and reason to
// realm_sync_socket_websocket_closed().
// The write_callback pointer does not need to be released by the CAPI implementation.
typedef void (*realm_sync_socket_websocket_async_write_func_t)(realm_userdata_t userdata,
                                                               realm_sync_socket_websocket_t websocket,
                                                               const char* data, size_t size,
                                                               realm_sync_socket_write_callback_t* write_callback);

// Called when the websocket has been destroyed in the Sync Client - no more write callbacks or observer
// functions should be called when this function is called.
typedef void (*realm_sync_socket_websocket_free_func_t)(realm_userdata_t userdata,
                                                        realm_sync_socket_websocket_t websocket);

/**
 * Get the VersionID of the current transaction.
 *
 * @param out_found True if version information is available. This requires an available Read or Write transaction.
 * @param out_version The version of the current transaction. If `out_found` returns False, this returns (0,0).
 * @return True if no exception occurred.
 */
RLM_API bool realm_get_version_id(const realm_t*, bool* out_found, realm_version_id_t* out_version);

/**
 * Get a string representing the version number of the Realm library.
 *
 * @return A null-terminated string.
 */
RLM_API const char* realm_get_library_version(void);

/**
 * Get individual components of the version number of the Realm library.
 *
 * @param out_major The major version number (X.0.0).
 * @param out_minor The minor version number (0.X.0).
 * @param out_patch The patch version number (0.0.X).
 * @param out_extra The extra version string (0.0.0-X).
 */
RLM_API void realm_get_library_version_numbers(int* out_major, int* out_minor, int* out_patch,
                                               const char** out_extra);

/**
 * Get the last error that happened on this thread.
 *
 * Errors are thread-local. Getting the error must happen on the same thread as
 * the call that caused the error to occur. The error is specific to the current
 * thread, and not the Realm instance for which the error occurred.
 *
 * Note: The error message in @a err will only be safe to use until the next API
 *       call is made on the current thread.
 *
 * Note: The error is not cleared by subsequent successful calls to this
 *       function, but it will be overwritten by subsequent failing calls to
 *       other library functions.
 *
 * Note: Calling this function does not clear the current last error.
 *
 * This function does not allocate any memory.
 *
 * @param err A pointer to a `realm_error_t` struct that will be populated with
 *            information about the last error, if there is one. May be NULL.
 * @return True if an error occurred.
 */
RLM_API bool realm_get_last_error(realm_error_t* err);

/**
 * Get information about an async error, potentially coming from another thread.
 *
 * This function does not allocate any memory.
 *
 * @param err A pointer to a `realm_error_t` struct that will be populated with
 *            information about the error. May not be NULL.
 * @return A bool indicating whether or not an error is available to be returned
 * @see realm_get_last_error()
 */
RLM_API bool realm_get_async_error(const realm_async_error_t* err, realm_error_t* out_err);

/**
 * Convert the last error to `realm_async_error_t`, which can safely be passed
 * between threads.
 *
 * Note: This function does not clear the last error.
 *
 * @return A non-null pointer if there was an error on this thread.
 * @see realm_get_last_error()
 * @see realm_get_async_error()
 * @see realm_clear_last_error()
 */
RLM_API realm_async_error_t* realm_get_last_error_as_async_error(void);

#if defined(__cplusplus)
/**
 * Invoke a function that may throw an exception, and report that exception as
 * part of the C API error handling mechanism.
 *
 * This is used to test translation of exceptions to error codes.
 *
 * @return True if no exception was thrown.
 */
RLM_EXPORT bool realm_wrap_exceptions(void (*)()) noexcept;
#endif // __cplusplus

/**
 * Clear the last error on the calling thread.
 *
 * Use this if the system has recovered from an error, e.g. by closing the
 * offending Realm and reopening it, freeing up resources, or similar.
 *
 * @return True if an error was cleared.
 */
RLM_API bool realm_clear_last_error(void);

/**
 * Free memory allocated by the module this library was linked into.
 *
 * This is needed for raw memory buffers such as string copies or arrays
 * returned from a library function. Realm C Wrapper objects on the other hand
 * should always be freed with realm_release() only.
 */
RLM_API void realm_free(void* buffer);

/**
 * Free any Realm C Wrapper object.
 *
 * Note: Any pointer returned from a library function is owned by the caller.
 *       The caller is responsible for calling `realm_release()`. The only
 *       exception from this is C++ bridge functions that return `void*`, with
 *       the prefix `_realm`.
 *
 * Note: C++ destructors are typically `noexcept`, so it is likely that an
 *       exception will crash the process.
 *
 * @param ptr A pointer to a Realm C Wrapper object. May be NULL.
 */
RLM_API void realm_release(void* ptr);

/**
 * Clone a Realm C Wrapper object.
 *
 * If the object is not clonable, this function fails with RLM_ERR_NOT_CLONABLE.
 *
 * @return A pointer to an object of the same type as the input, or NULL if
 *         cloning failed.
 */
RLM_API void* realm_clone(const void*);

/**
 * Return true if two API objects refer to the same underlying data. Objects
 * with different types are never equal.
 *
 * Note: This function cannot be used with types that have value semantics, only
 *       opaque types that have object semantics.
 *
 *    - `realm_t` objects are identical if they represent the same instance (not
 *      just if they represent the same file).
 *    - `realm_schema_t` objects are equal if the represented schemas are equal.
 *    - `realm_config_t` objects are equal if the configurations are equal.
 *    - `realm_object_t` objects are identical if they belong to the same realm
 *      and class, and have the same object key.
 *    - `realm_list_t` and other collection objects are identical if they come
 *      from the same object and property.
 *    - `realm_query_t` objects are never equal.
 *    - `realm_scheduler_t` objects are equal if they represent the same
 *      scheduler.
 *    - Query descriptor objects are equal if they represent equivalent
 *      descriptors.
 *    - `realm_async_error_t` objects are equal if they represent the same
 *      exception instance.
 *
 * This function cannot fail.
 */
RLM_API bool realm_equals(const void*, const void*);

/**
 * True if a Realm C Wrapper object is "frozen" (immutable).
 *
 * Objects, collections, and results can be frozen. For all other types, this
 * function always returns false.
 */
RLM_API bool realm_is_frozen(const void*);

/* Logging */
// equivalent to realm::util::Logger::Level in util/logger.hpp and must be kept in sync.
typedef enum realm_log_level {
    RLM_LOG_LEVEL_ALL = 0,
    RLM_LOG_LEVEL_TRACE = 1,
    RLM_LOG_LEVEL_DEBUG = 2,
    RLM_LOG_LEVEL_DETAIL = 3,
    RLM_LOG_LEVEL_INFO = 4,
    RLM_LOG_LEVEL_WARNING = 5,
    RLM_LOG_LEVEL_ERROR = 6,
    RLM_LOG_LEVEL_FATAL = 7,
    RLM_LOG_LEVEL_OFF = 8,
} realm_log_level_e;

typedef void (*realm_log_func_t)(realm_userdata_t userdata, const char* category, realm_log_level_e level,
                                 const char* message);

/**
 * Install the default logger
 */
RLM_API void realm_set_log_callback(realm_log_func_t, realm_userdata_t userdata,
                                    realm_free_userdata_func_t userdata_free) RLM_API_NOEXCEPT;
RLM_API void realm_set_log_level(realm_log_level_e) RLM_API_NOEXCEPT;
/**
 * Set the logging level for given category. Return the previous level.
 */
RLM_API realm_log_level_e realm_set_log_level_category(const char*, realm_log_level_e) RLM_API_NOEXCEPT;
/**
 * Get the logging level for given category.
 */
RLM_API realm_log_level_e realm_get_log_level_category(const char*) RLM_API_NOEXCEPT;
/**
 * Get the actual log category names (currently 15)
  @param num_values number of values in the out_values array
  @param out_values pointer to an array of size num_values
  @return returns the number of categories returned. If num_values is zero, it will
          return the total number of categories.
 */
RLM_API size_t realm_get_category_names(size_t num_values, const char** out_values);

/**
 * Get a thread-safe reference representing the same underlying object as some
 * API object.
 *
 * The thread safe reference can be passed to a different thread and resolved
 * against a different `realm_t` instance, which succeeds if the underlying
 * object still exists.
 *
 * The following types can produce thread safe references:
 *
 * - `realm_object_t`
 * - `realm_results_t`
 * - `realm_list_t`
 * - `realm_t`
 *
 * This does not assume ownership of the object, except for `realm_t`, where the
 * instance is transferred by value, and must be transferred back to the current
 * thread to be used. Note that the `realm_thread_safe_reference_t` object must
 * still be destroyed after having been converted into a `realm_t` object.
 *
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_thread_safe_reference_t* realm_create_thread_safe_reference(const void*);

/**
 * Allocate a new configuration with default options.
 */
RLM_API realm_config_t* realm_config_new(void);

/**
 * Get the path of the realm being opened.
 *
 * This function cannot fail.
 */
RLM_API const char* realm_config_get_path(const realm_config_t*);

/**
 * Set the path of the realm being opened.
 *
 * This function aborts when out of memory, but otherwise cannot fail.
 */
RLM_API void realm_config_set_path(realm_config_t*, const char* path);

/**
 * Get the encryption key for the realm.
 *
 * The output buffer must be at least 64 bytes.
 *
 * @returns The length of the encryption key (0 or 64)
 */
RLM_API size_t realm_config_get_encryption_key(const realm_config_t*, uint8_t* out_key);

/**
 * Set the encryption key for the realm.
 *
 * The key must be either 64 bytes long or have length zero (in which case
 * encryption is disabled).
 *
 * This function may fail if the encryption key has the wrong length.
 */
RLM_API bool realm_config_set_encryption_key(realm_config_t*, const uint8_t* key, size_t key_size);

/**
 * Get the schema for this realm.
 *
 * Note: The caller obtains ownership of the returned value, and must manually
 *       free it by calling `realm_release()`.
 *
 * @return A schema object, or NULL if the schema is not set (empty).
 */
RLM_API realm_schema_t* realm_config_get_schema(const realm_config_t*);

/**
 * Set the schema object for this realm.
 *
 * This does not take ownership of the schema object, and it should be released
 * afterwards.
 *
 * This function aborts when out of memory, but otherwise cannot fail.
 *
 * @param schema The schema object. May be NULL, which means an empty schema.
 */
RLM_API void realm_config_set_schema(realm_config_t*, const realm_schema_t* schema);

/**
 * Get the schema version of the schema.
 *
 * This function cannot fail.
 */
RLM_API uint64_t realm_config_get_schema_version(const realm_config_t*);

/**
 * Set the schema version of the schema.
 *
 * This function cannot fail.
 */
RLM_API void realm_config_set_schema_version(realm_config_t*, uint64_t version);

/**
 * Get the schema mode.
 *
 * This function cannot fail.
 */
RLM_API realm_schema_mode_e realm_config_get_schema_mode(const realm_config_t*);

/**
 * Set the schema mode.
 *
 * This function cannot fail.
 */
RLM_API void realm_config_set_schema_mode(realm_config_t*, realm_schema_mode_e);

/**
 * Get the subset schema mode.
 *
 * This function cannot fail.
 */
RLM_API realm_schema_subset_mode_e realm_config_get_schema_subset_mode(const realm_config_t*);

/**
 * Set schema subset mode
 *
 * This function cannot fail
 */
RLM_API void realm_config_set_schema_subset_mode(realm_config_t*, realm_schema_subset_mode_e);

/**
 * Set the migration callback.
 *
 * The migration function is called during a migration for schema modes
 * `RLM_SCHEMA_MODE_AUTOMATIC` and `RLM_SCHEMA_MODE_MANUAL`. The callback is
 * invoked with a realm instance before the migration and the realm instance
 * that is currently performing the migration.
 *
 * This function cannot fail.
 */
RLM_API void realm_config_set_migration_function(realm_config_t*, realm_migration_func_t, realm_userdata_t userdata,
                                                 realm_free_userdata_func_t userdata_free);

/**
 * Set the data initialization function.
 *
 * The callback is invoked the first time the schema is created, such that the
 * user can perform one-time initialization of the data in the realm.
 *
 * The realm instance passed to the callback is in a write transaction.
 *
 * This function cannot fail.
 */
RLM_API void realm_config_set_data_initialization_function(realm_config_t*, realm_data_initialization_func_t,
                                                           realm_userdata_t userdata,
                                                           realm_free_userdata_func_t userdata_free);

/**
 * Set the should-compact-on-launch callback.
 *
 * The callback is invoked the first time a realm file is opened in this process
 * to decide whether the realm file should be compacted.
 *
 * Note: If another process has the realm file open, it will not be compacted.
 *
 * This function cannot fail.
 */
RLM_API void realm_config_set_should_compact_on_launch_function(realm_config_t*,
                                                                realm_should_compact_on_launch_func_t,
                                                                realm_userdata_t userdata,
                                                                realm_free_userdata_func_t userdata_free);

/**
 * True if file format upgrades on open are disabled.
 *
 * This function cannot fail.
 */
RLM_API bool realm_config_get_disable_format_upgrade(const realm_config_t*);

/**
 * True if you can open the file without a file_format_upgrade
 */
RLM_API bool realm_config_needs_file_format_upgrade(const realm_config_t*);

/**
 * Disable file format upgrade on open (default: false).
 *
 * If a migration is needed to open the realm file with the provided schema, an
 * error is thrown rather than automatically performing the migration.
 *
 * This function cannot fail.
 */
RLM_API void realm_config_set_disable_format_upgrade(realm_config_t*, bool);

/**
 * True if automatic change notifications should be generated.
 *
 * This function cannot fail.
 */
RLM_API bool realm_config_get_automatic_change_notifications(const realm_config_t*);

/**
 * Automatically generated change notifications (default: true).
 *
 * This function cannot fail.
 */
RLM_API void realm_config_set_automatic_change_notifications(realm_config_t*, bool);

/**
 * The scheduler which this realm should be bound to (default: NULL).
 *
 * If NULL, the realm will be bound to the default scheduler for the current thread.
 *
 * This function aborts when out of memory, but otherwise cannot fail.
 */
RLM_API void realm_config_set_scheduler(realm_config_t*, const realm_scheduler_t*);

/**
 * Sync configuration for this realm (default: NULL).
 *
 * This function aborts when out of memory, but otherwise cannot fail.
 */
RLM_API void realm_config_set_sync_config(realm_config_t*, realm_sync_config_t*);

/**
 * Set the audit interface for the realm (unimplemented).
 */
RLM_API bool realm_config_set_audit_factory(realm_config_t*, void*);

/**
 * Get maximum number of active versions in the realm file allowed before an
 * exception is thrown.
 *
 * This function cannot fail.
 */
RLM_API uint64_t realm_config_get_max_number_of_active_versions(const realm_config_t*);

/**
 * Set maximum number of active versions in the realm file allowed before an
 * exception is thrown (default: UINT64_MAX).
 *
 * This function cannot fail.
 */
RLM_API void realm_config_set_max_number_of_active_versions(realm_config_t*, uint64_t);

/**
 * Configure realm to be in memory
 */
RLM_API void realm_config_set_in_memory(realm_config_t*, bool) RLM_API_NOEXCEPT;

/**
 * Check if realm is configured in memory
 */
RLM_API bool realm_config_get_in_memory(realm_config_t*) RLM_API_NOEXCEPT;

/**
 * Set FIFO path
 */
RLM_API void realm_config_set_fifo_path(realm_config_t*, const char*);

/**
 Check realm FIFO path
 */
RLM_API const char* realm_config_get_fifo_path(realm_config_t*) RLM_API_NOEXCEPT;

/**
 * If 'cached' is false, always return a new Realm instance.
 */
RLM_API void realm_config_set_cached(realm_config_t*, bool cached) RLM_API_NOEXCEPT;

/**
 * Check if realms are cached
 */
RLM_API bool realm_config_get_cached(realm_config_t*) RLM_API_NOEXCEPT;

/**
 * Allow realm to manage automatically embedded objects when a migration from TopLevel to Embedded takes place.
 */
RLM_API void realm_config_set_automatic_backlink_handling(realm_config_t*, bool) RLM_API_NOEXCEPT;

/**
 * Create a custom scheduler object from callback functions.
 *
 * @param notify Function which will be called whenever the scheduler has work
 *               to do. Each call to this should trigger a call to
 *               `realm_scheduler_perform_work()` from within the scheduler's
 *               event loop. This function must be thread-safe, or NULL, in
 *               which case the scheduler is considered unable to deliver
 *               notifications.
 * @param is_on_thread Function to return true if called from the same thread as
 *                     the scheduler. This function must be thread-safe.
 * @param can_deliver_notifications Function to return true if the scheduler can
 *                                  support `notify()`. This function does not
 *                                  need to be thread-safe.
 */
RLM_API realm_scheduler_t*
realm_scheduler_new(realm_userdata_t userdata, realm_free_userdata_func_t userdata_free,
                    realm_scheduler_notify_func_t notify, realm_scheduler_is_on_thread_func_t is_on_thread,
                    realm_scheduler_is_same_as_func_t is_same_as,
                    realm_scheduler_can_deliver_notifications_func_t can_deliver_notifications);

/**
 * Performs all of the pending work for the given scheduler.
 *
 * This function must be called from within the scheduler's event loop. It must
 * be called each time the notify callback passed to the scheduler
 * is invoked.
 */
RLM_API void realm_scheduler_perform_work(realm_work_queue_t*);
/**
 * Create an instance of the default scheduler for the current platform,
 * normally confined to the calling thread.
 */
RLM_API realm_scheduler_t* realm_scheduler_make_default(void);

/**
 * Get the scheduler used by frozen realms. This scheduler does not support
 * notifications, and does not perform any thread checking.
 *
 * This function is thread-safe, and cannot fail.
 */
RLM_API const realm_scheduler_t* realm_scheduler_get_frozen(void);

/**
 * Open a Realm file.
 *
 * @param config Realm configuration. If the Realm is already opened on another
 *               thread, validate that the given configuration is compatible
 *               with the existing one.
 * @return If successful, the Realm object. Otherwise, NULL.
 */
RLM_API realm_t* realm_open(const realm_config_t* config);

/**
 * The overloaded Realm::convert function offers a way to copy and/or convert a realm.
 *
 * The following options are supported:
 * - local -> local (config or path)
 * - local -> sync (config only)
 * - sync -> local (config only)
 * - sync -> sync  (config or path)
 * - sync -> bundlable sync (client file identifier removed)
 *
 * Note that for bundled realms it is required that all local changes are synchronized with the
 * server before the copy can be written. This is to be sure that the file can be used as a
 * stating point for a newly installed application. The function will throw if there are
 * pending uploads.
 */
/**
 * Copy or convert a Realm using a config.
 *
 * If the file already exists and merge_with_existing is true, data will be copied over object per object.
 * When merging, all classes must have a pk called '_id" otherwise an exception is thrown.
 * If the file exists and merge_with_existing is false, an exception is thrown.
 * If the file does not exist, the realm file will be exported to the new location and if the
 * configuration object contains a sync part, a sync history will be synthesized.
 *
 * @param config The realm configuration that should be used to create a copy.
 *               This can be a local or a synced Realm, encrypted or not.
 * @param merge_with_existing If this is true and the destination file exists, data will be copied over object by
 * object. Otherwise, if this is false and the destination file exists, an exception is thrown.
 */
RLM_API bool realm_convert_with_config(const realm_t* realm, const realm_config_t* config, bool merge_with_existing);
/**
 * Copy a Realm using a path.
 *
 * @param path The path the realm should be copied to. Local realms will remain local, synced
 *             realms will remain synced realms.
 * @param encryption_key The optional encryption key for the new realm.
 * @param merge_with_existing If this is true and the destination file exists, data will be copied over object by
 object.
 *  Otherwise, if this is false and the destination file exists, an exception is thrown.

 */
RLM_API bool realm_convert_with_path(const realm_t* realm, const char* path, realm_binary_t encryption_key,
                                     bool merge_with_existing);

/**
 * Deletes the following files for the given `realm_file_path` if they exist:
 * - the Realm file itself
 * - the .management folder
 * - the .note file
 * - the .log file
 *
 * The .lock file for this Realm cannot and will not be deleted as this is unsafe.
 * If a different process / thread is accessing the Realm at the same time a corrupt state
 * could be the result and checking for a single process state is not possible here.
 *
 * @param realm_file_path The path to the Realm file. All files will be derived from this.
 * @param[out] did_delete_realm If non-null, set to true if the primary Realm file was deleted.
 *                              Discard value if the function returns an error.
 *
 * @return true if no error occurred.
 *
 * @throws RLM_ERR_FILE_PERMISSION_DENIED if the operation was not permitted.
 * @throws RLM_ERR_FILE_ACCESS_ERROR for any other error while trying to delete the file or folder.
 * @throws RLM_ERR_DELETE_OPENED_REALM if the function was called on an open Realm.
 */
RLM_API bool realm_delete_files(const char* realm_file_path, bool* did_delete_realm);

/**
 * Create a `realm_t` object from a thread-safe reference to the same realm.
 *
 * @param tsr Thread-safe reference object created by calling
 *            `realm_get_thread_safe_reference()` with a `realm_t` instance.
 * @param scheduler The scheduler to use for the new `realm_t` instance. May be
 *                  NULL, in which case the default scheduler for the current
 *                  thread is used.
 * @return A non-null pointer if no error occurred.
 */
RLM_API realm_t* realm_from_thread_safe_reference(realm_thread_safe_reference_t* tsr, realm_scheduler_t* scheduler);

/**
 * Create a `realm_t*` from a `std::shared_ptr<Realm>*`.
 *
 * This is intended as a migration path for users of the C++ Object Store API.
 *
 * Call `realm_release()` on the returned `realm_t*` to decrement the refcount
 * on the inner `std::shared_ptr<Realm>`.
 *
 * @param pshared_ptr A pointer to an instance of `std::shared_ptr<Realm>`.
 * @param n Must be equal to `sizeof(std::shared_ptr<Realm>)`.
 * @return A `realm_t*` representing the same Realm object as the passed
 *         `std::shared_ptr<Realm>`.
 */
RLM_API realm_t* _realm_from_native_ptr(const void* pshared_ptr, size_t n);

/**
 * Get a `std::shared_ptr<Realm>` from a `realm_t*`.
 *
 * This is intended as a migration path for users of the C++ Object Store API.
 *
 * @param pshared_ptr A pointer to an instance of `std::shared_ptr<Realm>`.
 * @param n Must be equal to `sizeof(std::shared_ptr<Realm>)`.
 */
RLM_API void _realm_get_native_ptr(const realm_t*, void* pshared_ptr, size_t n);

/**
 * Forcibly close a Realm file.
 *
 * Note that this invalidates all Realm instances for the same path.
 *
 * The Realm will be automatically closed when the last reference is released,
 * including references to objects within the Realm.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_close(realm_t*);

/**
 * True if the Realm file is closed.
 *
 * This function cannot fail.
 */
RLM_API bool realm_is_closed(realm_t*);

/**
 * Begin a read transaction for the Realm file.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_begin_read(realm_t*);

/**
 * Begin a write transaction for the Realm file.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_begin_write(realm_t*);

/**
 * Return true if the realm is in a write transaction.
 *
 * This function cannot fail.
 */
RLM_API bool realm_is_writable(const realm_t*);

/**
 * Commit a write transaction.
 *
 * @return True if the commit succeeded and no exceptions were thrown.
 */
RLM_API bool realm_commit(realm_t*);

/**
 * Roll back a write transaction.
 *
 * @return True if the rollback succeeded and no exceptions were thrown.
 */
RLM_API bool realm_rollback(realm_t*);

/**
 * start a new write transaction asynchronously for the realm passed as argument.
 */
RLM_API bool realm_async_begin_write(realm_t* realm, realm_async_begin_write_func_t, realm_userdata_t userdata,
                                     realm_free_userdata_func_t userdata_free, bool notify_only,
                                     unsigned int* transaction_id);

/**
 * commit a transaction asynchronously for the realm passed as argument.
 */
RLM_API bool realm_async_commit(realm_t* realm, realm_async_commit_func_t, realm_userdata_t userdata,
                                realm_free_userdata_func_t userdata_free, bool allow_grouping,
                                unsigned int* transaction_id);

/**
 * Cancel the transaction referenced by the token passed as argument and set the optional boolean flag in order to
 * inform the caller if the transaction was cancelled.
 */
RLM_API bool realm_async_cancel(realm_t* realm, unsigned int token, bool* cancelled);

/**
 * Add a callback that will be invoked every time the view of this file is updated.
 *
 * This callback is guaranteed to be invoked before any object or collection change
 * notifications for this realm are delivered.
 *
 * @return a registration token used to remove the callback.
 */
RLM_API realm_callback_token_t* realm_add_realm_changed_callback(realm_t*, realm_on_realm_change_func_t,
                                                                 realm_userdata_t userdata,
                                                                 realm_free_userdata_func_t userdata_free);

/**
 * Add a callback that will be invoked the first time that the given realm is refreshed to the version which is the
 * latest version at the time when this is called.
 * @return a refresh token to remove the callback
 */
RLM_API realm_refresh_callback_token_t* realm_add_realm_refresh_callback(realm_t*, realm_on_realm_refresh_func_t,
                                                                         realm_userdata_t userdata,
                                                                         realm_free_userdata_func_t userdata_free);

/**
 * Refresh the view of the realm file.
 *
 * If another process or thread has made changes to the realm file, this causes
 * those changes to become visible in this realm instance.
 *
 * This calls `advance_read()` at the Core layer.
 *
 * @return True if no exceptions are thrown, false otherwise.
 */
RLM_API bool realm_refresh(realm_t*, bool* did_refresh);

/**
 * Produce a frozen view of this realm.
 *
 * @return A non-NULL realm instance representing the frozen state.
 */
RLM_API realm_t* realm_freeze(const realm_t*);

/**
 * Vacuum the free space from the realm file, reducing its file size.
 *
 * @return True if no exceptions are thrown, false otherwise.
 */
RLM_API bool realm_compact(realm_t*, bool* did_compact);

/**
 * Find and delete the table passed as parementer for the realm instance passed to this function.
 * @param table_name for the table the user wants to delete
 * @param table_deleted in order to indicate if the table was actually deleted from realm
 * @return true if no error has occurred, false otherwise
 */
RLM_API bool realm_remove_table(realm_t*, const char* table_name, bool* table_deleted);

/**
 * Create a new schema from classes and their properties.
 *
 * Note: This function does not validate the schema.
 *
 * Note: `realm_class_key_t` and `realm_property_key_t` values inside
 *       `realm_class_info_t` and `realm_property_info_t` are unused when
 *       defining the schema. Call `realm_get_schema()` to obtain the values for
 *       these fields in an open realm.
 *
 * @return True if allocation of the schema structure succeeded.
 */
RLM_API realm_schema_t* realm_schema_new(const realm_class_info_t* classes, size_t num_classes,
                                         const realm_property_info_t** class_properties);

/**
 * Get the schema for this realm.
 *
 * Note: The returned value is allocated by this function, so `realm_release()`
 *       must be called on it.
 */
RLM_API realm_schema_t* realm_get_schema(const realm_t*);

/**
 * Get the schema version for this realm.
 *
 * This function cannot fail.
 */
RLM_API uint64_t realm_get_schema_version(const realm_t* realm);

/**
 * Get the schema version for this realm at the path.
 */
RLM_API uint64_t realm_get_persisted_schema_version(const realm_config_t* config);

/**
 * Update the schema of an open realm.
 *
 * This is equivalent to calling `realm_update_schema_advanced(realm, schema, 0,
 * NULL, NULL, NULL, NULL, false)`.
 */
RLM_API bool realm_update_schema(realm_t* realm, const realm_schema_t* schema);

/**
 * Update the schema of an open realm, with options to customize certain steps
 * of the process.
 *
 * @param realm The realm for which the schema should be updated.
 * @param schema The new schema for the realm. If the schema is the same the
 *               existing schema, this function does nothing.
 * @param version The version of the new schema.
 * @param migration_func Callback to perform the migration. Has no effect if the
 *                       Realm is opened with `RLM_SCHEMA_MODE_ADDITIVE`.
 * @param migration_func_userdata Userdata pointer to pass to `migration_func`.
 * @param data_init_func Callback to perform initialization of the data in the
 *                       Realm if it is opened for the first time (i.e., it has
 *                       no previous schema version).
 * @param data_init_func_userdata Userdata pointer to pass to `data_init_func`.
 * @param is_in_transaction Pass true if the realm is already in a write
 *                          transaction. Otherwise, if the migration requires a
 *                          write transaction, this function will perform the
 *                          migration in its own write transaction.
 */
RLM_API bool realm_update_schema_advanced(realm_t* realm, const realm_schema_t* schema, uint64_t version,
                                          realm_migration_func_t migration_func,
                                          realm_userdata_t migration_func_userdata,
                                          realm_data_initialization_func_t data_init_func,
                                          realm_userdata_t data_init_func_userdata, bool is_in_transaction);

/**
 *  Rename a property for the schame  of the open realm.
 *  @param realm The realm for which the property schema has to be renamed
 *  @param schema The schema to modifies
 *  @param object_type type of the object to modify
 *  @param old_name old name of the property
 *  @param new_name new name of the property
 */
RLM_API bool realm_schema_rename_property(realm_t* realm, realm_schema_t* schema, const char* object_type,
                                          const char* old_name, const char* new_name);

/**
 * Get the `realm::Schema*` pointer for this realm.
 *
 * This is intended as a migration path for users of the C++ Object Store API.
 *
 * The returned value is owned by the `realm_t` instance, and must not be freed.
 */
RLM_API const void* _realm_get_schema_native(const realm_t*);

/**
 * Add a callback that will be invoked every time the schema of this realm is changed.
 *
 * @return a registration token used to remove the callback.
 */
RLM_API realm_callback_token_t* realm_add_schema_changed_callback(realm_t*, realm_on_schema_change_func_t,
                                                                  realm_userdata_t userdata,
                                                                  realm_free_userdata_func_t userdata_free);


/**
 * Validate the schema.
 *
 *  @param validation_mode A bitwise combination of values from the
 *                         enum realm_schema_validation_mode.
 *
 * @return True if the schema passed validation. If validation failed,
 *         `realm_get_last_error()` will produce an error describing the
 *         validation failure.
 */
RLM_API bool realm_schema_validate(const realm_schema_t*, uint64_t validation_mode);

/**
 * Return the number of classes in the Realm's schema.
 *
 * This cannot fail.
 */
RLM_API size_t realm_get_num_classes(const realm_t*);

/**
 * Get the table keys for classes in the schema.
 * In case of errors this function will return false (errors to be fetched via `realm_get_last_error()`).
 * If data is not copied the function will return true and set  `out_n` with the capacity needed.
 * Data is only copied if the input array has enough capacity, otherwise the needed  array capacity will be set.
 *
 * @param out_keys An array that will contain the keys of each class in the
 *                 schema. Array may be NULL, in this case no data will be copied and `out_n` set if not NULL.
 * @param max The maximum number of keys to write to `out_keys`.
 * @param out_n The actual number of classes. May be NULL.
 * @return True if no exception occurred.
 */
RLM_API bool realm_get_class_keys(const realm_t*, realm_class_key_t* out_keys, size_t max, size_t* out_n);

/**
 * Find a by the name of @a name.
 *
 * @param name The name of the class.
 * @param out_found Set to true if the class was found and no error occurred.
 *                  Otherwise, false. May not be NULL.
 * @param out_class_info A pointer to a `realm_class_info_t` that will be
 *                       populated with information about the class. May be
 *                       NULL.
 * @return True if no exception occurred.
 */
RLM_API bool realm_find_class(const realm_t*, const char* name, bool* out_found, realm_class_info_t* out_class_info);

/**
 * Get the class with @a key from the schema.
 *
 * Passing an invalid @a key for this schema is considered an error.
 *
 * @param key The key of the class, as discovered by `realm_get_class_keys()`.
 * @param out_class_info A pointer to a `realm_class_info_t` that will be
 *                       populated with the information of the class. May be
 *                       NULL, though that's kind of pointless.
 * @return True if no exception occurred.
 */
RLM_API bool realm_get_class(const realm_t*, realm_class_key_t key, realm_class_info_t* out_class_info);

/**
 * Get the list of properties for the class with this @a key.
 * In case of errors this function will return false (errors to be fetched via `realm_get_last_error()`).
 * If data is not copied the function will return true and set  `out_n` with the capacity needed.
 * Data is only copied if the input array has enough capacity, otherwise the needed  array capacity will be set.
 *
 * @param out_properties  A pointer to an array of `realm_property_info_t`, which
 *                       will be populated with the information about the
 *                       properties.  Array may be NULL, in this case no data will be copied and `out_n` set if not
 * NULL.
 * @param max The maximum number of entries to write to `out_properties`.
 * @param out_n The actual number of properties written to `out_properties`.
 * @return True if no exception occurred.
 */
RLM_API bool realm_get_class_properties(const realm_t*, realm_class_key_t key, realm_property_info_t* out_properties,
                                        size_t max, size_t* out_n);

/**
 * Get the property keys for the class with this @a key.
 * In case of errors this function will return false (errors to be fetched via `realm_get_last_error()`).
 * If data is not copied the function will return true and set  `out_n` with the capacity needed.
 * Data is only copied if the input array has enough capacity, otherwise the needed  array capacity will be set.
 *
 * @param key The class key.
 * @param out_col_keys An array of property keys. Array may be NULL,
 *                     in this case no data will be copied and `out_n` set if not NULL.
 * @param max The maximum number of keys to write to `out_col_keys`. Ignored if
 *            `out_col_keys == NULL`.
 * @param out_n The actual number of properties written to `out_col_keys` (if
 *              non-NULL), or number of properties in the class.
 * @return True if no exception occurred.
 **/
RLM_API bool realm_get_property_keys(const realm_t*, realm_class_key_t key, realm_property_key_t* out_col_keys,
                                     size_t max, size_t* out_n);

/**
 * Get the value for the property at the specified index in the object's schema.
 * @param prop_index The index of the property in the class properties array the realm was opened with.
 * @return True if no exception occurred.
 */
RLM_API bool realm_get_value_by_property_index(const realm_object_t* object, size_t prop_index,
                                               realm_value_t* out_value);

/**
 * Find a property by its column key.
 *
 * It is an error to pass a property @a key that is not present in this class.
 *
 * @param class_key The key of the class.
 * @param key The column key for the property.
 * @param out_property_info A pointer to a `realm_property_info_t` that will be
 *                          populated with information about the property.
 * @return True if no exception occurred.
 */
RLM_API bool realm_get_property(const realm_t*, realm_class_key_t class_key, realm_property_key_t key,
                                realm_property_info_t* out_property_info);

/**
 * Find a property by the internal (non-public) name of @a name.
 *
 * @param class_key The table key for the class.
 * @param name The name of the property.
 * @param out_found Will be set to true if the property was found. May not be
 *                  NULL.
 * @param out_property_info A pointer to a `realm_property_info_t` that will be
 *                          populated with information about the property. May
 *                          be NULL.
 * @return True if no exception occurred.
 */
RLM_API bool realm_find_property(const realm_t*, realm_class_key_t class_key, const char* name, bool* out_found,
                                 realm_property_info_t* out_property_info);

/**
 * Find a property with the public name of @a name.
 *
 * @param class_key The table key for the class.
 * @param public_name The public name of the property.
 * @param out_found Will be set to true if the property was found. May not be
 *                  NULL.
 * @param out_property_info A pointer to a `realm_property_info_t` that will be
 *                          populated with information about the property. May
 *                          be NULL.
 * @return True if no exception occurred.
 */
RLM_API bool realm_find_property_by_public_name(const realm_t*, realm_class_key_t class_key, const char* public_name,
                                                bool* out_found, realm_property_info_t* out_property_info);

/**
 * Find the primary key property for a class, if it has one.
 *
 * @param class_key The table key for this class.
 * @param out_found Will be set to true if the property was found. May not be
 *                  NULL.
 * @param out_property_info A property to a `realm_property_info_t` that will be
 *                          populated with information about the property, if it
 *                          was found. May be NULL.
 * @return True if no exception occurred.
 */
RLM_API bool realm_find_primary_key_property(const realm_t*, realm_class_key_t class_key, bool* out_found,
                                             realm_property_info_t* out_property_info);

/**
 * Get the number of objects in a table (class).
 *
 * @param out_count A pointer to a `size_t` that will contain the number of
 *                  objects, if successful.
 * @return True if the table key was valid for this realm.
 */
RLM_API bool realm_get_num_objects(const realm_t*, realm_class_key_t, size_t* out_count);

/**
 * Get the number of versions found in the Realm file.
 *
 * @param out_versions_count A pointer to a `size_t` that will contain the number of
 *                           versions, if successful.
 * @return True if no exception occurred.
 */
RLM_API bool realm_get_num_versions(const realm_t*, uint64_t* out_versions_count);

/**
 * Get an object with a particular object key.
 *
 * @param class_key The class key.
 * @param obj_key The key to the object. Passing a non-existent key is
 *                considered an error.
 * @return A non-NULL pointer if no exception occurred.
 */
RLM_API realm_object_t* realm_get_object(const realm_t*, realm_class_key_t class_key, realm_object_key_t obj_key);

/**
 * Get the parent object for the object passed as argument. Only works for embedded objects.
 * @return true, if no errors occurred.
 */
RLM_API bool realm_object_get_parent(const realm_object_t* object, realm_object_t** parent,
                                     realm_class_key_t* class_key);

/**
 * Find an object with a particular primary key value.
 *
 * @param out_found A pointer to a boolean that will be set to true or false if
 *                  no error occurred.
 * @return A non-NULL pointer if the object was found and no exception occurred.
 */
RLM_API realm_object_t* realm_object_find_with_primary_key(const realm_t*, realm_class_key_t, realm_value_t pk,
                                                           bool* out_found);

/**
 * Find all objects in class.
 *
 * Note: This is faster than running a query matching all objects (such as
 *       "TRUEPREDICATE").
 *
 * @return A non-NULL pointer if no exception was thrown.
 */
RLM_API realm_results_t* realm_object_find_all(const realm_t*, realm_class_key_t);

/**
 * Create an object in a class without a primary key.
 *
 * @return A non-NULL pointer if the object was created successfully.
 */
RLM_API realm_object_t* realm_object_create(realm_t*, realm_class_key_t);

/**
 * Create an object in a class with a primary key. Will not succeed if an
 * object with the given primary key value already exists.
 *
 * @return A non-NULL pointer if the object was created successfully.
 */
RLM_API realm_object_t* realm_object_create_with_primary_key(realm_t*, realm_class_key_t, realm_value_t pk);

/**
 * Create an object in a class with a primary key. If an object with the given
 * primary key value already exists, that object will be returned.
 *
 * @return A non-NULL pointer if the object was found/created successfully.
 */
RLM_API realm_object_t* realm_object_get_or_create_with_primary_key(realm_t*, realm_class_key_t, realm_value_t pk,
                                                                    bool* did_create);

/**
 * Delete a realm object.
 *
 * Note: This does not call `realm_release()` on the `realm_object_t` instance.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_object_delete(realm_object_t*);

/**
 * Resolve the Realm object in the provided Realm.
 *
 * This is equivalent to producing a thread-safe reference and resolving it in the target realm.
 *
 * If the object can be resolved in the target realm, '*resolved' points to the new object
 * If the object cannot be resolved in the target realm, '*resolved' will be null.
 * @return True if no exception occurred (except exceptions that may normally occur if resolution fails)
 */
RLM_API bool realm_object_resolve_in(const realm_object_t* live_object, const realm_t* target_realm,
                                     realm_object_t** resolved);

/**
 * Increment atomically property specified as parameter by value, for the object passed as argument.
 * @param object valid ptr to an object store in the database
 * @param property_key id of the property to change
 * @param value increment for the property passed as argument
 * @return True if not exception occurred.
 */
RLM_API bool realm_object_add_int(realm_object_t* object, realm_property_key_t property_key, int64_t value);


RLM_API realm_object_t* _realm_object_from_native_copy(const void* pobj, size_t n);
RLM_API realm_object_t* _realm_object_from_native_move(void* pobj, size_t n);
RLM_API const void* _realm_object_get_native_ptr(realm_object_t*);

/**
 * True if this object still exists in the realm.
 *
 * This function cannot fail.
 */
RLM_API bool realm_object_is_valid(const realm_object_t*);

/**
 * Get the key for this object.
 *
 * This function cannot fail.
 */
RLM_API realm_object_key_t realm_object_get_key(const realm_object_t* object);

/**
 * Get the table for this object.
 *
 * This function cannot fail.
 */
RLM_API realm_class_key_t realm_object_get_table(const realm_object_t* object);

/**
 * Get a `realm_link_t` representing a link to @a object.
 *
 * This function cannot fail.
 */
RLM_API realm_link_t realm_object_as_link(const realm_object_t* object);

/**
 * Helper method for making it easier to to convert SDK input to the underlying
 * `realm_key_path_array_t`.
 *
 * @return A pointer to the converted key path array. NULL in case of an error.
 */
RLM_API realm_key_path_array_t* realm_create_key_path_array(const realm_t* realm,
                                                            const realm_class_key_t object_class_key,
                                                            size_t num_key_paths, const char** user_key_paths);

/**
 * Subscribe to notifications for this object.
 *
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_notification_token_t* realm_object_add_notification_callback(realm_object_t*, realm_userdata_t userdata,
                                                                           realm_free_userdata_func_t userdata_free,
                                                                           realm_key_path_array_t* key_path_array,
                                                                           realm_on_object_change_func_t on_change);

/**
 * Get an object from a thread-safe reference, potentially originating in a
 * different `realm_t` instance
 */
RLM_API realm_object_t* realm_object_from_thread_safe_reference(const realm_t*, realm_thread_safe_reference_t*);

/**
 * Get the value for a property.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_get_value(const realm_object_t*, realm_property_key_t, realm_value_t* out_value);

/**
 * Get the values for several properties.
 *
 * This is provided as an alternative to calling `realm_get_value()` multiple
 * times in a row, which is particularly useful for language runtimes where
 * crossing the native bridge is comparatively expensive. In addition, it
 * eliminates some parameter validation that would otherwise be repeated for
 * each call.
 *
 * Example use cases:
 *
 *  - Extracting all properties of an object for serialization.
 *  - Converting an object to some in-memory representation.
 *
 * @param num_values The number of elements in @a properties and @a out_values.
 * @param properties The keys for the properties to fetch. May not be NULL.
 * @param out_values Where to write the property values. If an error occurs,
 *                   this array may only be partially initialized. May not be
 *                   NULL.
 * @return True if no exception occurs.
 */
RLM_API bool realm_get_values(const realm_object_t*, size_t num_values, const realm_property_key_t* properties,
                              realm_value_t* out_values);

/**
 * Set the value for a property.
 *
 * @param new_value The new value for the property.
 * @param is_default True if this property is being set as part of setting the
 *                   default values for a new object. This has no effect in
 *                   non-sync'ed realms.
 * @return True if no exception occurred.
 */
RLM_API bool realm_set_value(realm_object_t*, realm_property_key_t, realm_value_t new_value, bool is_default);

/**
 * Assign a JSON formatted string to a Mixed property. Underlying structures will be created as needed
 *
 * @param json_string The new value for the property.
 * @return True if no exception occurred.
 */
RLM_API bool realm_set_json(realm_object_t*, realm_property_key_t, const char* json_string);

/**
 * Create an embedded object in a given property.
 *
 * @return A non-NULL pointer if the object was created successfully.
 */
RLM_API realm_object_t* realm_set_embedded(realm_object_t*, realm_property_key_t);

/**
 * Create a collection in a given Mixed property.
 *
 */
RLM_API realm_list_t* realm_set_list(realm_object_t*, realm_property_key_t);
RLM_API realm_dictionary_t* realm_set_dictionary(realm_object_t*, realm_property_key_t);

/** Return the object linked by the given property
 *
 * @return A non-NULL pointer if an object is found.
 */
RLM_API realm_object_t* realm_get_linked_object(realm_object_t*, realm_property_key_t);

/**
 * Serializes an object to json and returns it as string. Serializes a single level of properties only.
 *
 * @return a json-serialized representation of the object.
 */
RLM_API char* realm_object_to_string(realm_object_t*);

/**
 * Set the values for several properties.
 *
 * This is provided as an alternative to calling `realm_get_value()` multiple
 * times in a row, which is particularly useful for language runtimes where
 * crossing the native bridge is comparatively expensive. In addition, it
 * eliminates some parameter validation that would otherwise be repeated for
 * each call.
 *
 * Example use cases:
 *
 *  - Initializing a new object with default values.
 *  - Deserializing some in-memory structure into a realm object.
 *
 * This operation is "atomic"; if an exception occurs due to invalid input (such
 * as type mismatch, nullability mismatch, etc.), the object will remain
 * unmodified.
 *
 * @param num_values The number of elements in @a properties and @a values.
 * @param properties The keys of the properties to set. May not be NULL.
 * @param values The values to assign to the properties. May not be NULL.
 * @param is_default True if the properties are being set as part of setting
 *                   default values for a new object. This has no effect in
 *                   non-sync'ed realms.
 * @return True if no exception occurred.
 */
RLM_API bool realm_set_values(realm_object_t*, size_t num_values, const realm_property_key_t* properties,
                              const realm_value_t* values, bool is_default);

/**
 * Get a list instance for the property of an object.
 *
 * Note: It is up to the caller to call `realm_release()` on the returned list.
 *
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_list_t* realm_get_list(realm_object_t*, realm_property_key_t);

/**
 * Create a `realm_list_t` from a pointer to a `realm::List`, copy-constructing
 * the internal representation.
 *
 * @param plist A pointer to an instance of `realm::List`.
 * @param n Must be equal to `sizeof(realm::List)`.
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_list_t* _realm_list_from_native_copy(const void* plist, size_t n);

/**
 * Create a `realm_list_t` from a pointer to a `realm::List`, move-constructing
 * the internal representation.
 *
 * @param plist A pointer to an instance of `realm::List`.
 * @param n Must be equal to `sizeof(realm::List)`.
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_list_t* _realm_list_from_native_move(void* plist, size_t n);

/**
 * Resolve the list in the context of a given Realm instance.
 *
 * This is equivalent to producing a thread-safe reference and resolving it in the frozen realm.
 *
 * If resolution is possible, a valid resolved object is produced at '*resolved*'.
 * If resolution is not possible, but no error occurs, '*resolved' is set to NULL
 *
 * @return true if no error occurred.
 */
RLM_API bool realm_list_resolve_in(const realm_list_t* list, const realm_t* target_realm, realm_list_t** resolved);

/**
 * Check if a list is valid.
 *
 * @return True if the list is valid.
 */
RLM_API bool realm_list_is_valid(const realm_list_t*);

/**
 * Get the size of a list, in number of elements.
 *
 * This function may fail if the object owning the list has been deleted.
 *
 * @param out_size Where to put the list size. May be NULL.
 * @return True if no exception occurred.
 */
RLM_API bool realm_list_size(const realm_list_t*, size_t* out_size);

/**
 * Get the property that this list came from.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_list_get_property(const realm_list_t*, realm_property_info_t* out_property_info);

/**
 * Get the value at @a index.
 *
 * @param out_value The resulting value, if no error occurred. May be NULL,
 *                  though nonsensical.
 * @return True if no exception occurred.
 */
RLM_API bool realm_list_get(const realm_list_t*, size_t index, realm_value_t* out_value);

/**
 * Find the value in the list passed as parameter.
 * @param value to search in the list
 * @param out_index the index in the list where the value has been found or realm::not_found.
 * @param out_found boolean that indicates whether the value is found or not
 * @return true if no exception occurred.
 */
RLM_API bool realm_list_find(const realm_list_t*, const realm_value_t* value, size_t* out_index, bool* out_found);

/**
 * Set the value at @a index.
 *
 * @param value The value to set.
 * @return True if no exception occurred.
 */
RLM_API bool realm_list_set(realm_list_t*, size_t index, realm_value_t value);

/**
 * Insert @a value at @a index.
 *
 * @param value The value to insert.
 * @return True if no exception occurred.
 */
RLM_API bool realm_list_insert(realm_list_t*, size_t index, realm_value_t value);

/**
 * Insert a collection inside a list (only available for mixed types)
 *
 * @param list valid ptr to a list of mixed
 * @param index position in the list where to add the collection
 * @return pointer to a valid collection that has been just inserted at the index passed as argument
 */
RLM_API realm_list_t* realm_list_insert_list(realm_list_t* list, size_t index);
RLM_API realm_dictionary_t* realm_list_insert_dictionary(realm_list_t* list, size_t index);

/**
 * Set a collection inside a list (only available for mixed types).
 * If the list already contains a collection of the requested type, the
 * operation is idempotent.
 *
 * @param list valid ptr to a list where a nested collection needs to be set
 * @param index position in the list where to set the collection
 * @return a valid ptr representing the collection just set
 */
RLM_API realm_list_t* realm_list_set_list(realm_list_t* list, size_t index);
RLM_API realm_dictionary_t* realm_list_set_dictionary(realm_list_t* list, size_t index);

/**
 * Returns a nested list if such collection exists, NULL otherwise.
 *
 * @param list pointer to the list that containes the nested list
 * @param index index of collection in the list
 * @return a pointer to the the nested list found at the index passed as argument
 */
RLM_API realm_list_t* realm_list_get_list(realm_list_t* list, size_t index);

/**
 * Returns a nested dictionary if such collection exists, NULL otherwise.
 *
 * @param list pointer to the list that containes the nested collection into
 * @param index position of collection in the list
 * @return a pointer to the the nested dictionary found at index passed as argument
 */
RLM_API realm_dictionary_t* realm_list_get_dictionary(realm_list_t* list, size_t index);

/**
 * Move the element at @a from_index to @a to_index.
 *
 * @param from_index The index of the element to move.
 * @param to_index The index to move the element to.
 * @return True if no exception occurred.
 */
RLM_API bool realm_list_move(realm_list_t*, size_t from_index, size_t to_index);

/**
 * Insert an embedded object at a given position.
 *
 * @return A non-NULL pointer if the object was created successfully.
 */
RLM_API realm_object_t* realm_list_insert_embedded(realm_list_t*, size_t index);

/**
 * Create an embedded object at a given position.
 *
 * @return A non-NULL pointer if the object was created successfully.
 */
RLM_API realm_object_t* realm_list_set_embedded(realm_list_t*, size_t index);

/**
 * Get object identified at index
 *
 * @return A non-NULL pointer if value is an object.
 */
RLM_API realm_object_t* realm_list_get_linked_object(realm_list_t*, size_t index);

/**
 * Erase the element at @a index.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_list_erase(realm_list_t*, size_t index);

/**
 * Clear a list, removing all elements in the list. In a list of links, this
 * does *NOT* delete the target objects.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_list_clear(realm_list_t*);

/**
 * In a list of objects, delete all objects in the list and clear the list. In a
 * list of values, clear the list.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_list_remove_all(realm_list_t*);

/**
 * Subscribe to notifications for this object.
 *
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_notification_token_t* realm_list_add_notification_callback(realm_list_t*, realm_userdata_t userdata,
                                                                         realm_free_userdata_func_t userdata_free,
                                                                         realm_key_path_array_t* key_path_array,
                                                                         realm_on_collection_change_func_t on_change);

/**
 * Get an list from a thread-safe reference, potentially originating in a
 * different `realm_t` instance
 */
RLM_API realm_list_t* realm_list_from_thread_safe_reference(const realm_t*, realm_thread_safe_reference_t*);

/**
 * True if an object notification indicates that the object was deleted.
 *
 * This function cannot fail.
 */
RLM_API bool realm_object_changes_is_deleted(const realm_object_changes_t*);

/**
 * Get the number of properties that were modified in an object notification.
 *
 * This function cannot fail.
 */
RLM_API size_t realm_object_changes_get_num_modified_properties(const realm_object_changes_t*);

/**
 * Get the column keys for the properties that were modified in an object
 * notification.
 *
 * This function cannot fail.
 *
 * @param out_modified Where the column keys should be written. May be NULL.
 * @param max The maximum number of column keys to write.
 * @return The number of column keys written to @a out_modified, or the number
 *         of modified properties if @a out_modified is NULL.
 */
RLM_API size_t realm_object_changes_get_modified_properties(const realm_object_changes_t*,
                                                            realm_property_key_t* out_modified, size_t max);

/**
 * Get the number of various types of changes in a collection notification.
 *
 * @param out_num_deletions The number of deletions. May be NULL.
 * @param out_num_insertions The number of insertions. May be NULL.
 * @param out_num_modifications The number of modifications. May be NULL.
 * @param out_num_moves The number of moved elements. May be NULL.
 * @param out_collection_was_cleared a flag to signal if the collection has been cleared. May be NULL
 * @param out_collection_was_deleted a flag to signal if the collection has been deleted. May be NULL
 */
RLM_API void realm_collection_changes_get_num_changes(const realm_collection_changes_t*, size_t* out_num_deletions,
                                                      size_t* out_num_insertions, size_t* out_num_modifications,
                                                      size_t* out_num_moves, bool* out_collection_was_cleared,
                                                      bool* out_collection_was_deleted);

/**
 * Get the number of various types of changes in a collection notification,
 * suitable for acquiring the change indices as ranges, which is much more
 * compact in memory than getting the individual indices when multiple adjacent
 * elements have been modified.
 *
 * @param out_num_deletion_ranges The number of deleted ranges. May be NULL.
 * @param out_num_insertion_ranges The number of inserted ranges. May be NULL.
 * @param out_num_modification_ranges The number of modified ranges. May be
 *                                    NULL.
 * @param out_num_moves The number of moved elements. May be NULL.
 */
RLM_API void realm_collection_changes_get_num_ranges(const realm_collection_changes_t*,
                                                     size_t* out_num_deletion_ranges,
                                                     size_t* out_num_insertion_ranges,
                                                     size_t* out_num_modification_ranges, size_t* out_num_moves);
typedef struct realm_collection_move {
    size_t from;
    size_t to;
} realm_collection_move_t;

typedef struct realm_index_range {
    size_t from;
    size_t to;
} realm_index_range_t;

/**
 * Get the indices of changes in a collection notification.
 *
 * Note: For moves, every `from` index will also be present among deletions, and
 *       every `to` index will also be present among insertions.
 *
 * This function cannot fail.
 *
 * @param out_deletion_indices Where to put the indices of deleted elements
 *                             (*before* the deletion happened). May be NULL.
 * @param max_deletion_indices The max number of indices to write to @a
 *                             out_deletion_indices.
 * @param out_insertion_indices Where the put the indices of inserted elements
 *                              (*after* the insertion happened). May be NULL.
 * @param max_insertion_indices The max number of indices to write to @a
 *                              out_insertion_indices.
 * @param out_modification_indices Where to put the indices of modified elements
 *                                 (*before* any insertions or deletions of
 *                                 other elements). May be NULL.
 * @param max_modification_indices The max number of indices to write to @a
 *                                 out_modification_indices.
 * @param out_modification_indices_after Where to put the indices of modified
 *                                       elements (*after* any insertions or
 *                                       deletions of other elements). May be
 *                                       NULL.
 * @param max_modification_indices_after The max number of indices to write to
 *                                       @a out_modification_indices_after.
 * @param out_moves Where to put the pairs of indices of moved elements. May be
 *                  NULL.
 * @param max_moves The max number of pairs to write to @a out_moves.
 */
RLM_API void realm_collection_changes_get_changes(const realm_collection_changes_t*, size_t* out_deletion_indices,
                                                  size_t max_deletion_indices, size_t* out_insertion_indices,
                                                  size_t max_insertion_indices, size_t* out_modification_indices,
                                                  size_t max_modification_indices,
                                                  size_t* out_modification_indices_after,
                                                  size_t max_modification_indices_after,
                                                  realm_collection_move_t* out_moves, size_t max_moves);

RLM_API void realm_collection_changes_get_ranges(
    const realm_collection_changes_t*, realm_index_range_t* out_deletion_ranges, size_t max_deletion_ranges,
    realm_index_range_t* out_insertion_ranges, size_t max_insertion_ranges,
    realm_index_range_t* out_modification_ranges, size_t max_modification_ranges,
    realm_index_range_t* out_modification_ranges_after, size_t max_modification_ranges_after,
    realm_collection_move_t* out_moves, size_t max_moves);

/**
 * Returns the number of changes occurred to the dictionary passed as argument
 *
 * @param changes valid ptr to the dictionary changes structure
 * @param out_deletions_size number of deletions
 * @param out_insertion_size number of insertions
 * @param out_modification_size number of modifications
 * @param out_was_deleted a flag to signal if the dictionary has been deleted.
 */
RLM_API void realm_dictionary_get_changes(const realm_dictionary_changes_t* changes, size_t* out_deletions_size,
                                          size_t* out_insertion_size, size_t* out_modification_size,
                                          bool* out_was_deleted);

/**
 * Returns the list of keys changed for the dictionary passed as argument.
 * The user must assure that there is enough memory to accomodate all the keys
 * calling `realm_dictionary_get_changes` before.
 *
 * @param changes valid ptr to the dictionary changes structure
 * @param deletions list of deleted keys
 * @param deletions_size size of the list of deleted keys
 * @param insertions list of inserted keys
 * @param insertions_size size of the list of inserted keys
 * @param modifications list of modified keys
 * @param modification_size size of the list of modified keys
 * @param collection_was_cleared whether or not the collection was cleared
 */
RLM_API void realm_dictionary_get_changed_keys(const realm_dictionary_changes_t* changes, realm_value_t* deletions,
                                               size_t* deletions_size, realm_value_t* insertions,
                                               size_t* insertions_size, realm_value_t* modifications,
                                               size_t* modification_size, bool* collection_was_cleared);

/**
 * Get a set instance for the property of an object.
 *
 * Note: It is up to the caller to call `realm_release()` on the returned set.
 *
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_set_t* realm_get_set(realm_object_t*, realm_property_key_t);

/**
 * Create a `realm_set_t` from a pointer to a `realm::object_store::Set`,
 * copy-constructing the internal representation.
 *
 * @param pset A pointer to an instance of `realm::object_store::Set`.
 * @param n Must be equal to `sizeof(realm::object_store::Set)`.
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_set_t* _realm_set_from_native_copy(const void* pset, size_t n);

/**
 * Create a `realm_set_t` from a pointer to a `realm::object_store::Set`,
 * move-constructing the internal representation.
 *
 * @param pset A pointer to an instance of `realm::object_store::Set`.
 * @param n Must be equal to `sizeof(realm::object_store::Set)`.
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_set_t* _realm_set_from_native_move(void* pset, size_t n);

/**
 * Resolve the set in the context of a given Realm instance.
 *
 * This is equivalent to producing a thread-safe reference and resolving it in the frozen realm.
 *
 * If resolution is possible, a valid resolved object is produced at '*resolved*'.
 * If resolution is not possible, but no error occurs, '*resolved' is set to NULL
 *
 * @return true if no error occurred.
 */
RLM_API bool realm_set_resolve_in(const realm_set_t* list, const realm_t* target_realm, realm_set_t** resolved);

/**
 * Check if a set is valid.
 *
 * @return True if the set is valid.
 */
RLM_API bool realm_set_is_valid(const realm_set_t*);

/**
 * Get the size of a set, in number of unique elements.
 *
 * This function may fail if the object owning the set has been deleted.
 *
 * @param out_size Where to put the set size. May be NULL.
 * @return True if no exception occurred.
 */
RLM_API bool realm_set_size(const realm_set_t*, size_t* out_size);

/**
 * Get the property that this set came from.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_set_get_property(const realm_set_t*, realm_property_info_t* out_property_info);

/**
 * Get the value at @a index.
 *
 * Note that elements in a set move around arbitrarily when other elements are
 * inserted/removed.
 *
 * @param out_value The resulting value, if no error occurred. May be NULL,
 *                  though nonsensical.
 * @return True if no exception occurred.
 */
RLM_API bool realm_set_get(const realm_set_t*, size_t index, realm_value_t* out_value);

/**
 * Find an element in a set.
 *
 * If @a value has a type that is incompatible with the set, it will be reported
 * as not existing in the set.
 *
 * @param value The value to look for in the set.
 * @param out_index If non-null, and the element is found, this will be
 *                  populated with the index of the found element in the set.
 * @param out_found If non-null, will be set to true if the element was found,
 *                  otherwise will be set to false.
 * @return True if no exception occurred.
 */
RLM_API bool realm_set_find(const realm_set_t*, realm_value_t value, size_t* out_index, bool* out_found);

/**
 * Insert an element in a set.
 *
 * If the element is already in the set, this function does nothing (and does
 * not report an error).
 *
 * @param value The value to insert.
 * @param out_index If non-null, will be set to the index of the inserted
 *                  element, or the index of the existing element.
 * @param out_inserted If non-null, will be set to true if the element did not
 *                     already exist in the set. Otherwise set to false.
 * @return True if no exception occurred.
 */
RLM_API bool realm_set_insert(realm_set_t*, realm_value_t value, size_t* out_index, bool* out_inserted);

/**
 * Erase an element from a set.
 *
 * If the element does not exist in the set, this function does nothing (and
 * does not report an error).
 *
 * @param value The value to erase.
 * @param out_erased If non-null, will be set to true if the element was found
 *                   and erased, and otherwise set to false.
 * @return True if no exception occurred.
 */
RLM_API bool realm_set_erase(realm_set_t*, realm_value_t value, bool* out_erased);

/**
 * Clear a set of values.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_set_clear(realm_set_t*);

/**
 * In a set of objects, delete all objects in the set and clear the set. In a
 * set of values, clear the set.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_set_remove_all(realm_set_t*);

/**
 * Subscribe to notifications for this object.
 *
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_notification_token_t* realm_set_add_notification_callback(realm_set_t*, realm_userdata_t userdata,
                                                                        realm_free_userdata_func_t userdata_free,
                                                                        realm_key_path_array_t* key_path_array,
                                                                        realm_on_collection_change_func_t on_change);
/**
 * Get an set from a thread-safe reference, potentially originating in a
 * different `realm_t` instance
 */
RLM_API realm_set_t* realm_set_from_thread_safe_reference(const realm_t*, realm_thread_safe_reference_t*);

/**
 * Get a dictionary instance for the property of an object.
 *
 * Note: It is up to the caller to call `realm_release()` on the returned dictionary.
 *
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_dictionary_t* realm_get_dictionary(realm_object_t*, realm_property_key_t);

/**
 * Create a `realm_dictionary_t` from a pointer to a `realm::object_store::Dictionary`,
 * copy-constructing the internal representation.
 *
 * @param pdict A pointer to an instance of `realm::object_store::Dictionary`.
 * @param n Must be equal to `sizeof(realm::object_store::Dictionary)`.
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_dictionary_t* _realm_dictionary_from_native_copy(const void* pdict, size_t n);

/**
 * Create a `realm_dictionary_t` from a pointer to a `realm::object_store::Dictionary`,
 * move-constructing the internal representation.
 *
 * @param pdict A pointer to an instance of `realm::object_store::Dictionary`.
 * @param n Must be equal to `sizeof(realm::object_store::Dictionary)`.
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_dictionary_t* _realm_dictionary_from_native_move(void* pdict, size_t n);

/**
 * Resolve the list in the context of a given Realm instance.
 *
 * This is equivalent to producing a thread-safe reference and resolving it in the frozen realm.
 *
 * If resolution is possible, a valid resolved object is produced at '*resolved*'.
 * If resolution is not possible, but no error occurs, '*resolved' is set to NULL
 *
 * @return true if no error occurred.
 */
RLM_API bool realm_dictionary_resolve_in(const realm_dictionary_t* list, const realm_t* target_realm,
                                         realm_dictionary_t** resolved);

/**
 * Check if a list is valid.
 *
 * @return True if the list is valid.
 */
RLM_API bool realm_dictionary_is_valid(const realm_dictionary_t*);

/**
 * Get the size of a dictionary (the number of unique keys).
 *
 * This function may fail if the object owning the dictionary has been deleted.
 *
 * @param out_size Where to put the dictionary size. May be NULL.
 * @return True if no exception occurred.
 */
RLM_API bool realm_dictionary_size(const realm_dictionary_t*, size_t* out_size);


/**
 * Get the property that this dictionary came from.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_dictionary_get_property(const realm_dictionary_t*, realm_property_info_t* out_info);

/**
 * Find an element in a dictionary.
 *
 * @param key The key to look for.
 * @param out_value If non-null, the value for the corresponding key.
 * @param out_found If non-null, will be set to true if the dictionary contained the key.
 * @return True if no exception occurred.
 */
RLM_API bool realm_dictionary_find(const realm_dictionary_t*, realm_value_t key, realm_value_t* out_value,
                                   bool* out_found);

/**
 * Get the key-value pair at @a index.
 *
 * Note that the indices of elements in the dictionary move around as other
 * elements are inserted/removed.
 *
 * @param index The index in the dictionary.
 * @param out_key If non-null, will be set to the key at the corresponding index.
 * @param out_value If non-null, will be set to the value at the corresponding index.
 * @return True if no exception occurred.
 */
RLM_API bool realm_dictionary_get(const realm_dictionary_t*, size_t index, realm_value_t* out_key,
                                  realm_value_t* out_value);

/**
 * Insert or update an element in a dictionary.
 *
 * If the key already exists, the value will be overwritten.
 *
 * @param key The lookup key.
 * @param value The value to insert.
 * @param out_index If non-null, will be set to the index of the element after
 *                  insertion/update.
 * @param out_inserted If non-null, will be set to true if the key did not
 *                     already exist.
 * @return True if no exception occurred.
 */
RLM_API bool realm_dictionary_insert(realm_dictionary_t*, realm_value_t key, realm_value_t value, size_t* out_index,
                                     bool* out_inserted);

/**
 * Insert an embedded object.
 *
 * @return A non-NULL pointer if the object was created successfully.
 */
RLM_API realm_object_t* realm_dictionary_insert_embedded(realm_dictionary_t*, realm_value_t key);

/**
 * Insert a collection inside a dictionary (only available for mixed types)
 *
 * @param dictionary valid ptr to a dictionary of mixed
 * @param key the mixed representing a key for a dictionary (only string)
 * @return pointer to a valid collection that has been just inserted at the key passed as argument
 */
RLM_API realm_list_t* realm_dictionary_insert_list(realm_dictionary_t* dictionary, realm_value_t key);
RLM_API realm_dictionary_t* realm_dictionary_insert_dictionary(realm_dictionary_t*, realm_value_t);


/**
 * Fetch a list from a dictionary.
 * @return a valid list that needs to be deleted by the caller or nullptr in case of an error.
 */
RLM_API realm_list_t* realm_dictionary_get_list(realm_dictionary_t* dictionary, realm_value_t key);

/**
 * Fetch a dictioanry from a dictionary.
 * @return a valid dictionary that needs to be deleted by the caller or nullptr in case of an error.
 */
RLM_API realm_dictionary_t* realm_dictionary_get_dictionary(realm_dictionary_t* dictionary, realm_value_t key);

/**
 * Get object identified by key
 *
 * @return A non-NULL pointer if the value associated with key is an object.
 */
RLM_API realm_object_t* realm_dictionary_get_linked_object(realm_dictionary_t*, realm_value_t key);

/**
 * Erase a dictionary element.
 *
 * @param key The key of the element to erase.
 * @param out_erased If non-null, will be set to true if the element was found
 *                   and erased.
 * @return True if no exception occurred.
 */
RLM_API bool realm_dictionary_erase(realm_dictionary_t*, realm_value_t key, bool* out_erased);

/**
 * Return the list of keys stored in the dictionary
 *
 * @param out_size number of keys
 * @param out_keys the list of keys in the dictionary, the memory has to be released once it is no longer used.
 * @return True if no exception occurred.
 */
RLM_API bool realm_dictionary_get_keys(realm_dictionary_t*, size_t* out_size, realm_results_t** out_keys);

/**
 * Check if the dictionary contains a certain key
 *
 * @param key to search in the dictionary
 * @param found True if the such key exists
 * @return True if no exception occurred
 */
RLM_API bool realm_dictionary_contains_key(const realm_dictionary_t*, realm_value_t key, bool* found);

/**
 * Check if the dictionary contains a certain value
 *
 * @param value to search in the dictionary
 * @param index the index of the value in the dictionry if such value exists
 * @return True if no exception occurred
 */
RLM_API bool realm_dictionary_contains_value(const realm_dictionary_t*, realm_value_t value, size_t* index);


/**
 * Clear a dictionary.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_dictionary_clear(realm_dictionary_t*);

/**
 * Subscribe to notifications for this object.
 *
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_notification_token_t* realm_dictionary_add_notification_callback(
    realm_dictionary_t*, realm_userdata_t userdata, realm_free_userdata_func_t userdata_free,
    realm_key_path_array_t* key_path_array, realm_on_dictionary_change_func_t on_change);

/**
 * Get an dictionary from a thread-safe reference, potentially originating in a
 * different `realm_t` instance
 */
RLM_API realm_dictionary_t* realm_dictionary_from_thread_safe_reference(const realm_t*,
                                                                        realm_thread_safe_reference_t*);

/**
 * Parse a query string and bind it to a table.
 *
 * If the query failed to parse, the parser error is available from
 * `realm_get_last_error()`.
 *
 * @param target_table The table on which to run this query.
 * @param query_string A zero-terminated string in the Realm Query Language,
 *                     optionally containing argument placeholders (`$0`, `$1`,
 *                     etc.).
 * @param num_args The number of arguments for this query.
 * @param args A pointer to a list of argument values.
 * @return A non-null pointer if the query was successfully parsed and no
 *         exception occurred.
 */
RLM_API realm_query_t* realm_query_parse(const realm_t*, realm_class_key_t target_table, const char* query_string,
                                         size_t num_args, const realm_query_arg_t* args);


/**
 * Get textual representation of query
 *
 * @return a string containing the description. The string memory is managed by the query object.
 */
RLM_API const char* realm_query_get_description(realm_query_t*);


/**
 * Parse a query string and append it to an existing query via logical &&.
 * The query string applies to the same table and Realm as the existing query.
 *
 * If the query failed to parse, the parser error is available from
 * `realm_get_last_error()`.
 *
 * @param query_string A zero-terminated string in the Realm Query Language,
 *                     optionally containing argument placeholders (`$0`, `$1`,
 *                     etc.).
 * @param num_args The number of arguments for this query.
 * @param args A pointer to a list of argument values.
 * @return A non-null pointer if the query was successfully parsed and no
 *         exception occurred.
 */
RLM_API realm_query_t* realm_query_append_query(const realm_query_t*, const char* query_string, size_t num_args,
                                                const realm_query_arg_t* args);

/**
 * Parse a query string and bind it to a list.
 *
 * If the query failed to parse, the parser error is available from
 * `realm_get_last_error()`.
 *
 * @param target_list The list on which to run this query.
 * @param query_string A string in the Realm Query Language, optionally
 *                     containing argument placeholders (`$0`, `$1`, etc.).
 * @param num_args The number of arguments for this query.
 * @param args A pointer to a list of argument values.
 * @return A non-null pointer if the query was successfully parsed and no
 *         exception occurred.
 */
RLM_API realm_query_t* realm_query_parse_for_list(const realm_list_t* target_list, const char* query_string,
                                                  size_t num_args, const realm_query_arg_t* args);

/**
 * Parse a query string and bind it to a set.
 *
 * If the query failed to parse, the parser error is available from
 * `realm_get_last_error()`.
 *
 * @param target_set The set on which to run this query.
 * @param query_string A string in the Realm Query Language, optionally
 *                     containing argument placeholders (`$0`, `$1`, etc.).
 * @param num_args The number of arguments for this query.
 * @param args A pointer to a list of argument values.
 * @return A non-null pointer if the query was successfully parsed and no
 *         exception occurred.
 */
RLM_API realm_query_t* realm_query_parse_for_set(const realm_set_t* target_set, const char* query_string,
                                                 size_t num_args, const realm_query_arg_t* args);
/**
 * Parse a query string and bind it to another query result.
 *
 * If the query failed to parse, the parser error is available from
 * `realm_get_last_error()`.
 *
 * @param target_results The results on which to run this query.
 * @param query_string A zero-terminated string in the Realm Query Language,
 *                     optionally containing argument placeholders (`$0`, `$1`,
 *                     etc.).
 * @param num_args The number of arguments for this query.
 * @param args A pointer to a list of argument values.
 * @return A non-null pointer if the query was successfully parsed and no
 *         exception occurred.
 */
RLM_API realm_query_t* realm_query_parse_for_results(const realm_results_t* target_results, const char* query_string,
                                                     size_t num_args, const realm_query_arg_t* args);

/**
 * Count the number of objects found by this query.
 */
RLM_API bool realm_query_count(const realm_query_t*, size_t* out_count);

/**
 * Return the first object matched by this query.
 *
 * Note: This function can only produce objects, not values. Use the
 *       `realm_results_t` returned by `realm_query_find_all()` to retrieve
 *       values from a list of primitive values.
 *
 * @param out_value Where to write the result, if any object matched the query.
 *                  May be NULL.
 * @param out_found Where to write whether the object was found. May be NULL.
 * @return True if no exception occurred.
 */
RLM_API bool realm_query_find_first(realm_query_t*, realm_value_t* out_value, bool* out_found);

/**
 * Produce a results object for this query.
 *
 * Note: This does not actually run the query until the results are accessed in
 *       some way.
 *
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_results_t* realm_query_find_all(realm_query_t*);

/**
 * Convert a list to results.
 *
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_results_t* realm_list_to_results(realm_list_t*);

/**
 * Convert a set to results.
 *
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_results_t* realm_set_to_results(realm_set_t*);

/**
 * Convert a dictionary to results.
 *
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_results_t* realm_dictionary_to_results(realm_dictionary_t*);

/**
 * Fetch the backlinks for the object passed as argument.
 * @return a valid ptr to realm results that contains all the backlinks for the object, or null in case of errors.
 */
RLM_API realm_results_t* realm_get_backlinks(realm_object_t* object, realm_class_key_t source_table_key,
                                             realm_property_key_t property_key);

/**
 * Delete all objects matched by a query.
 */
RLM_API bool realm_query_delete_all(const realm_query_t*);

/**
 * Set the boolean passed as argument to true or false whether the realm_results passed is valid or not
 * @return true/false if no exception has occurred.
 */
RLM_API bool realm_results_is_valid(const realm_results_t*, bool*);

/**
 * Count the number of results.
 *
 * If the result is "live" (not a snapshot), this may rerun the query if things
 * have changed.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_results_count(realm_results_t*, size_t* out_count);

/**
 * Create a new results object by further filtering existing result.
 *
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_results_t* realm_results_filter(realm_results_t*, realm_query_t*);

/**
 * Create a new results object by further sorting existing result.
 *
 * @param sort_string Specifies a sort condition. It has the format
          <param> ["," <param>]*
          <param> ::= <prop> ["." <prop>]* <direction>,
          <direction> ::= "ASCENDING" | "DESCENDING"
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_results_t* realm_results_sort(realm_results_t* results, const char* sort_string);

/**
 * Create a new results object by removing duplicates
 *
 * @param distinct_string Specifies a distinct condition. It has the format
          <param> ["," <param>]*
          <param> ::= <prop> ["." <prop>]*
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_results_t* realm_results_distinct(realm_results_t* results, const char* distinct_string);

/**
 * Create a new results object by limiting the number of items
 *
 * @param max_count Specifies the number of elements the new result can have at most
 * @return A non-null pointer if no exception occurred.
 */
RLM_API realm_results_t* realm_results_limit(realm_results_t* results, size_t max_count);

/**
 * Get the matching element at @a index in the results.
 *
 * If the result is "live" (not a snapshot), this may rerun the query if things
 * have changed.
 *
 * Note: The bound returned by `realm_results_count()` for a non-snapshot result
 *       is not a reliable way to iterate over elements in the result, because
 *       the result will be live-updated if changes are made in each iteration
 *       that may change the number of query results or even change the
 *       ordering. In other words, this method should probably only be used with
 *       snapshot results.
 *
 * @return True if no exception occurred (including out-of-bounds).
 */
RLM_API bool realm_results_get(realm_results_t*, size_t index, realm_value_t* out_value);

/**
 * Returns an instance of realm_list at the index passed as argument.
 * @return A valid ptr to a list instance or nullptr in case of errors
 */
RLM_API realm_list_t* realm_results_get_list(realm_results_t*, size_t index);

/**
 * Returns an instance of realm_dictionary for the index passed as argument.
 * @return A valid ptr to a dictionary instance or nullptr in case of errors
 */
RLM_API realm_dictionary_t* realm_results_get_dictionary(realm_results_t*, size_t index);

/**
 * Find the index for the value passed as parameter inside realm results pointer passed a input parameter.
 *  @param value the value to find inside the realm results
 *  @param out_index the index where the object has been found, or realm::not_found
 *  @param out_found boolean indicating if the value has been found or not
 *  @return true if no error occurred, false otherwise
 */
RLM_API bool realm_results_find(realm_results_t*, realm_value_t* value, size_t* out_index, bool* out_found);

/**
 * Get the matching object at @a index in the results.
 *
 * If the result is "live" (not a snapshot), this may rerun the query if things
 * have changed.
 *
 * Note: The bound returned by `realm_results_count()` for a non-snapshot result
 *       is not a reliable way to iterate over elements in the result, because
 *       the result will be live-updated if changes are made in each iteration
 *       that may change the number of query results or even change the
 *       ordering. In other words, this method should probably only be used with
 *       snapshot results.
 *
 * @return An instance of `realm_object_t` if no exception occurred.
 */
RLM_API realm_object_t* realm_results_get_object(realm_results_t*, size_t index);

/**
 * Return the query associated to the results passed as argument.
 *
 * @param results the ptr to a valid results object.
 * @return a valid ptr to realm_query_t if no error has occurred
 */
RLM_API realm_query_t* realm_results_get_query(realm_results_t* results);

/**
 * Find the index for the realm object passed as parameter inside realm results pointer passed a input parameter.
 *  @param value the value to find inside the realm results
 *  @param out_index the index where the object has been found, or realm::not_found
 *  @param out_found boolean indicating if the value has been found or not
 *  @return true if no error occurred, false otherwise
 */
RLM_API bool realm_results_find_object(realm_results_t*, realm_object_t* value, size_t* out_index, bool* out_found);

/**
 * Delete all objects in the result.
 *
 * If the result if "live" (not a snapshot), this may rerun the query if things
 * have changed.
 *
 * @return True if no exception occurred.
 */
RLM_API bool realm_results_delete_all(realm_results_t*);

/**
 * Return a snapshot of the results that never automatically updates.
 *
 * The returned result is suitable for use with `realm_results_count()` +
 * `realm_results_get()`.
 */
RLM_API realm_results_t* realm_results_snapshot(const realm_results_t*);

/**
 * Map the Results into a live Realm instance.
 *
 * This is equivalent to producing a thread-safe reference and resolving it in the live realm.
 *
 * @return A live copy of the Results.
 */
RLM_API realm_results_t* realm_results_resolve_in(realm_results_t* from_results, const realm_t* target_realm);

/**
 * Compute the minimum value of a property in the results.
 *
 * @param out_min Where to write the result, if there were matching rows.
 * @param out_found Set to true if there are matching rows.
 * @return True if no exception occurred.
 */
RLM_API bool realm_results_min(realm_results_t*, realm_property_key_t, realm_value_t* out_min, bool* out_found);

/**
 * Compute the maximum value of a property in the results.
 *
 * @param out_max Where to write the result, if there were matching rows.
 * @param out_found Set to true if there are matching rows.
 * @return True if no exception occurred.
 */
RLM_API bool realm_results_max(realm_results_t*, realm_property_key_t, realm_value_t* out_max, bool* out_found);

/**
 * Compute the sum value of a property in the results.
 *
 * @param out_sum Where to write the result. Zero if no rows matched.
 * @param out_found Set to true if there are matching rows.
 * @return True if no exception occurred.
 */
RLM_API bool realm_results_sum(realm_results_t*, realm_property_key_t, realm_value_t* out_sum, bool* out_found);

/**
 * Compute the average value of a property in the results.
 *
 * Note: For numeric columns, the average is always converted to double.
 *
 * @param out_average Where to write the result.
 * @param out_found Set to true if there are matching rows.
 * @return True if no exception occurred.
 */
RLM_API bool realm_results_average(realm_results_t*, realm_property_key_t, realm_value_t* out_average,
                                   bool* out_found);

RLM_API realm_notification_token_t* realm_results_add_notification_callback(realm_results_t*,
                                                                            realm_userdata_t userdata,
                                                                            realm_free_userdata_func_t userdata_free,
                                                                            realm_key_path_array_t* key_path_array,
                                                                            realm_on_collection_change_func_t);

/**
 * Get an results object from a thread-safe reference, potentially originating
 * in a different `realm_t` instance
 */
RLM_API realm_results_t* realm_results_from_thread_safe_reference(const realm_t*, realm_thread_safe_reference_t*);

/* HTTP transport */
typedef enum realm_http_request_method {
    RLM_HTTP_REQUEST_METHOD_GET,
    RLM_HTTP_REQUEST_METHOD_POST,
    RLM_HTTP_REQUEST_METHOD_PATCH,
    RLM_HTTP_REQUEST_METHOD_PUT,
    RLM_HTTP_REQUEST_METHOD_DELETE,
} realm_http_request_method_e;

typedef struct realm_http_header {
    const char* name;
    const char* value;
} realm_http_header_t;

typedef struct realm_http_request {
    realm_http_request_method_e method;
    const char* url;
    uint64_t timeout_ms;
    const realm_http_header_t* headers;
    size_t num_headers;
    const char* body;
    size_t body_size;
} realm_http_request_t;

typedef struct realm_http_response {
    int status_code;
    int custom_status_code;
    const realm_http_header_t* headers;
    size_t num_headers;
    const char* body;
    size_t body_size;
} realm_http_response_t;

/**
 * Callback function used by Core to make a HTTP request.
 *
 * Complete the request by calling realm_http_transport_complete_request(),
 * passing in the request_context pointer here and the received response.
 * Network request are expected to be asynchronous and can be completed on any thread.
 *
 * @param request The request to send.
 * @param request_context Internal state pointer of Core, needed by realm_http_transport_complete_request().
 */
typedef void (*realm_http_request_func_t)(realm_userdata_t userdata, const realm_http_request_t request,
                                          void* request_context);

typedef struct realm_http_transport realm_http_transport_t;

/**
 * Create a new HTTP transport with these callbacks implementing its functionality.
 */
RLM_API realm_http_transport_t* realm_http_transport_new(realm_http_request_func_t, realm_userdata_t userdata,
                                                         realm_free_userdata_func_t userdata_free);

/**
 * Complete a HTTP request with the given response.
 *
 * @param request_context Internal state pointer passed by Core when invoking realm_http_request_func_t
 *                        to start the request.
 * @param response The server response to the HTTP request initiated by Core.
 */
RLM_API void realm_http_transport_complete_request(void* request_context, const realm_http_response_t* response);

/* App */
typedef struct realm_user realm_user_t;
typedef enum realm_user_state {
    RLM_USER_STATE_LOGGED_OUT,
    RLM_USER_STATE_LOGGED_IN,
    RLM_USER_STATE_REMOVED
} realm_user_state_e;

// This type should never be returned from a function.
// It's only meant as an asynchronous callback argument.
// Pointers to this struct and its pointer members are only valid inside the scope
// of the callback they were passed to.
typedef struct realm_app_error {
    realm_errno_e error;
    realm_error_categories categories;
    const char* message;

    /**
     * The underlying HTTP status code returned by the server,
     * otherwise zero.
     */
    int http_status_code;

    /**
     * A link to MongoDB Realm server logs related to the error,
     * or NULL if error response didn't contain log information.
     */
    const char* link_to_server_logs;
} realm_app_error_t;

#if REALM_APP_SERVICES
typedef struct realm_app realm_app_t;
typedef struct realm_app_credentials realm_app_credentials_t;

typedef enum realm_auth_provider {
    RLM_AUTH_PROVIDER_ANONYMOUS,
    RLM_AUTH_PROVIDER_ANONYMOUS_NO_REUSE,
    RLM_AUTH_PROVIDER_FACEBOOK,
    RLM_AUTH_PROVIDER_GOOGLE,
    RLM_AUTH_PROVIDER_APPLE,
    RLM_AUTH_PROVIDER_CUSTOM,
    RLM_AUTH_PROVIDER_EMAIL_PASSWORD,
    RLM_AUTH_PROVIDER_FUNCTION,
    RLM_AUTH_PROVIDER_API_KEY,
} realm_auth_provider_e;

typedef enum realm_sync_client_metadata_mode {
    RLM_SYNC_CLIENT_METADATA_MODE_PLAINTEXT,
    RLM_SYNC_CLIENT_METADATA_MODE_ENCRYPTED,
    RLM_SYNC_CLIENT_METADATA_MODE_DISABLED,
} realm_sync_client_metadata_mode_e;

typedef struct realm_app_user_apikey {
    realm_object_id_t id;
    const char* key;
    const char* name;
    bool disabled;
} realm_app_user_apikey_t;

typedef struct realm_user_identity {
    /**
     * Ptr to null terminated string representing user identity (memory has to be freed by SDK)
     */
    char* id;
    /**
     * Enum representing the list of auth providers
     */
    realm_auth_provider_e provider_type;
} realm_user_identity_t;

typedef void (*realm_return_apikey_func_t)(realm_userdata_t userdata, realm_app_user_apikey_t*,
                                           const realm_app_error_t*);
typedef void (*realm_return_apikey_list_func_t)(realm_userdata_t userdata, realm_app_user_apikey_t[], size_t count,
                                                realm_app_error_t*);

typedef void (*realm_return_string_func_t)(realm_userdata_t userdata, const char* serialized_ejson_response,
                                           const realm_app_error_t*);
/**
 * Generic completion callback for asynchronous Realm App operations.
 *
 * @param error Pointer to an error object if the operation failed, otherwise null if it completed successfully.
 */
typedef void (*realm_app_void_completion_func_t)(realm_userdata_t userdata, const realm_app_error_t* error);

/**
 * Completion callback for asynchronous Realm App operations that yield a user object.
 *
 * @param user User object produced by the operation, or null if it failed.
 *             The pointer is alive only for the duration of the callback,
 *             if you wish to use it further make a copy with realm_clone().
 * @param error Pointer to an error object if the operation failed, otherwise null if it completed successfully.
 */
typedef void (*realm_app_user_completion_func_t)(realm_userdata_t userdata, realm_user_t* user,
                                                 const realm_app_error_t* error);

RLM_API realm_app_credentials_t* realm_app_credentials_new_anonymous(bool reuse_credentials) RLM_API_NOEXCEPT;
RLM_API realm_app_credentials_t* realm_app_credentials_new_facebook(const char* access_token) RLM_API_NOEXCEPT;
RLM_API realm_app_credentials_t* realm_app_credentials_new_google_id_token(const char* id_token) RLM_API_NOEXCEPT;
RLM_API realm_app_credentials_t* realm_app_credentials_new_google_auth_code(const char* auth_code) RLM_API_NOEXCEPT;
RLM_API realm_app_credentials_t* realm_app_credentials_new_apple(const char* id_token) RLM_API_NOEXCEPT;
RLM_API realm_app_credentials_t* realm_app_credentials_new_jwt(const char* jwt_token) RLM_API_NOEXCEPT;
RLM_API realm_app_credentials_t* realm_app_credentials_new_email_password(const char* email,
                                                                          realm_string_t password) RLM_API_NOEXCEPT;
RLM_API realm_app_credentials_t* realm_app_credentials_new_api_key(const char* api_key) RLM_API_NOEXCEPT;

/**
 * Create Custom Function authentication app credentials.
 *
 * @param serialized_ejson_payload The arguments array to invoke the function with,
 *                                 serialized as an Extended JSON string.
 * @return null, if an error occurred.
 */
RLM_API realm_app_credentials_t* realm_app_credentials_new_function(const char* serialized_ejson_payload);

RLM_API realm_auth_provider_e realm_auth_credentials_get_provider(realm_app_credentials_t*) RLM_API_NOEXCEPT;

/**
 * Create a new app configuration.
 *
 * @param app_id The MongoDB Realm app id.
 * @param http_transport The HTTP transport used to make network calls.
 */
RLM_API realm_app_config_t* realm_app_config_new(const char* app_id,
                                                 const realm_http_transport_t* http_transport) RLM_API_NOEXCEPT;

RLM_API const char* realm_app_get_default_base_url(void) RLM_API_NOEXCEPT;
RLM_API void realm_app_config_set_base_url(realm_app_config_t*, const char*) RLM_API_NOEXCEPT;
RLM_API void realm_app_config_set_default_request_timeout(realm_app_config_t*, uint64_t ms) RLM_API_NOEXCEPT;
RLM_API void realm_app_config_set_platform_version(realm_app_config_t*, const char*) RLM_API_NOEXCEPT;
RLM_API void realm_app_config_set_sdk_version(realm_app_config_t*, const char*) RLM_API_NOEXCEPT;
RLM_API void realm_app_config_set_sdk(realm_app_config_t* config, const char* sdk) RLM_API_NOEXCEPT;
RLM_API void realm_app_config_set_device_name(realm_app_config_t* config, const char* device_name) RLM_API_NOEXCEPT;
RLM_API void realm_app_config_set_device_version(realm_app_config_t* config,
                                                 const char* device_version) RLM_API_NOEXCEPT;
RLM_API void realm_app_config_set_framework_name(realm_app_config_t* config,
                                                 const char* framework_name) RLM_API_NOEXCEPT;
RLM_API void realm_app_config_set_framework_version(realm_app_config_t* config,
                                                    const char* framework_version) RLM_API_NOEXCEPT;
RLM_API void realm_app_config_set_bundle_id(realm_app_config_t* config, const char* bundle_id) RLM_API_NOEXCEPT;
RLM_API void realm_app_config_set_base_file_path(realm_app_config_t*, const char*) RLM_API_NOEXCEPT;
RLM_API void realm_app_config_set_metadata_mode(realm_app_config_t*,
                                                realm_sync_client_metadata_mode_e) RLM_API_NOEXCEPT;
RLM_API void realm_app_config_set_metadata_encryption_key(realm_app_config_t*, const uint8_t[64]) RLM_API_NOEXCEPT;
RLM_API void realm_app_config_set_security_access_group(realm_app_config_t*, const char*) RLM_API_NOEXCEPT;

/**
 * Get an existing @a realm_app_credentials_t and return it's json representation
 * Note: the caller must delete the pointer to the string via realm_release
 *
 * @return: a non-null ptr to the string representing the json configuration.
 */
RLM_API const char* realm_app_credentials_serialize_as_json(realm_app_credentials_t*) RLM_API_NOEXCEPT;

/**
 * Create realm_app_t* instance given a valid realm app configuration.
 *
 * @return A non-null pointer if no error occurred.
 */
RLM_API realm_app_t* realm_app_create(const realm_app_config_t*);

/**
 * Create cached realm_app_t* instance given a valid realm app configuration.
 *
 * @return A non-null pointer if no error occurred.
 */
RLM_API realm_app_t* realm_app_create_cached(const realm_app_config_t*);

/**
 * Get a cached realm_app_t* instance given an app id. out_app may be null if the app with this id hasn't been
 * previously cached by calling realm_app_create_cached.
 *
 * @return true if no error occurred.
 */
RLM_API bool realm_app_get_cached(const char* app_id, const char* base_url, realm_app_t** out_app);

/**
 * Clear all the cached @a realm_app_t* instances in the process.
 *
 * @a realm_app_t* instances will need to be disposed with realm_release()
 * for them to be fully destroyed after the cache is cleared.
 */
RLM_API void realm_clear_cached_apps(void) RLM_API_NOEXCEPT;

RLM_API const char* realm_app_get_app_id(const realm_app_t*) RLM_API_NOEXCEPT;
RLM_API realm_user_t* realm_app_get_current_user(const realm_app_t*) RLM_API_NOEXCEPT;

/**
 * Update the URL used to communicate with the Realm server. This function will update the location
 * information used for http and websocket requests to the server. Once this operation has completed,
 * the new base_url value returned by realm_app_get_base_url() will match the base_url value provided
 * to this function. Any App requests performed while the base URl update is currently in progress
 * will continue to use the original base URL value.
 *
 * @param app ptr to realm_app
 * @param base_url The new base URL value to set as the Realm server URL - a null or empty string will
 *                 use the default base URL value
 * @param callback invoked once operation has completed
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_update_base_url(realm_app_t* app, const char* base_url,
                                       realm_app_void_completion_func_t callback, realm_userdata_t userdata,
                                       realm_free_userdata_func_t userdata_free);

/**
 * Return the current base URL value used by the app. If the realm_app_update_base_url() is called, this
 * value will match the base_url value provided to that function when the update is complete. The value
 * provided by this function is undefined if the realm_app_update_base_url() operation is in progress,
 * since it will likely be the base_url value prior to realm_app_update_base_url() being called.
 *
 * @param app ptr to realm_app
 * @return The current base URL string used by the app
 *
 * Return value must be manually released with realm_free().
 */
RLM_API char* realm_app_get_base_url(realm_app_t* app) RLM_API_NOEXCEPT;

/**
 * Get the list of active users in this @a app.
 * In case of errors this function will return false (errors to be fetched via `realm_get_last_error()`).
 * If data is not copied the function will return true and set  `out_n` with the capacity needed.
 * Data is only copied if the input array has enough capacity, otherwise the needed  array capacity will be set.
 *
 * @param out_users A pointer to an array of `realm_user_t*`, which
 *                  will be populated with the list of active users in the app.
 *                  Array may be NULL, in this case no data will be copied and `out_n` set if not NULL.
 * @param capacity The maximum number of elements `out_users` can hold.
 * @param out_n The actual number of entries written to `out_users`.
 *              May be NULL.
 * @return True if no exception occurred.
 */
RLM_API bool realm_app_get_all_users(const realm_app_t* app, realm_user_t** out_users, size_t capacity,
                                     size_t* out_n);

/**
 * Log in a user and asynchronously retrieve a user object. Inform caller via callback once operation completes.
 * @param app ptr to realm_app
 * @param credentials sync credentials
 * @param callback invoked once operation has completed
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_log_in_with_credentials(realm_app_t* app, realm_app_credentials_t* credentials,
                                               realm_app_user_completion_func_t callback, realm_userdata_t userdata,
                                               realm_free_userdata_func_t userdata_free);

/**
 * Logout the current user.
 * @param app ptr to realm_app
 * @param callback invoked once operation has completed
 * @param userdata custom userdata ptr
 * @param userdata_free deleter for custom userdata
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_log_out_current_user(realm_app_t* app, realm_app_void_completion_func_t callback,
                                            void* userdata, realm_free_userdata_func_t userdata_free);

/**
 * Refreshes the custom data for a specified user.
 * @param app ptr to realm_app
 * @param user ptr to user
 * @param callback invoked once operation has completed
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_refresh_custom_data(realm_app_t* app, realm_user_t* user,
                                           realm_app_void_completion_func_t callback, realm_userdata_t userdata,
                                           realm_free_userdata_func_t userdata_free);

/**
 * Log out the given user if they are not already logged out.
 * @param app ptr to realm_app
 * @param user ptr to user
 * @param callback invoked once operation has completed
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_log_out(realm_app_t* app, realm_user_t* user, realm_app_void_completion_func_t callback,
                               realm_userdata_t userdata, realm_free_userdata_func_t userdata_free);

/**
 * Links the currently authenticated user with a new identity, where the identity is defined by the credentia
 * specified as a parameter.
 * @param app ptr to realm_app
 * @param user ptr to the user to link
 * @param credentials sync credentials
 * @param callback invoked once operation has completed
 * @param userdata custom userdata ptr
 * @param userdata_free deleter for custom userdata
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_link_user(realm_app_t* app, realm_user_t* user, realm_app_credentials_t* credentials,
                                 realm_app_user_completion_func_t callback, void* userdata,
                                 realm_free_userdata_func_t userdata_free);

/**
 * Switches the active user with the specified one. The user must exist in the list of all users who have logged into
 * this application.
 * @param app ptr to realm_app
 * @param user ptr to user to set as current.
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_switch_user(realm_app_t* app, realm_user_t* user);

/**
 * Logs out and removes the provided user.
 * @param app ptr to realm_app
 * @param user ptr to the user to remove
 * @param callback invoked once operation has completed
 * @param userdata custom userdata ptr
 * @param userdata_free deleter for custom userdata
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_remove_user(realm_app_t* app, realm_user_t* user, realm_app_void_completion_func_t callback,
                                   void* userdata, realm_free_userdata_func_t userdata_free);

/**
 * Deletes a user and all its data from the server.
 * @param app ptr to realm_app
 * @param user ptr to the user to delete
 * @param callback invoked once operation has completed
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_delete_user(realm_app_t* app, realm_user_t* user, realm_app_void_completion_func_t callback,
                                   realm_userdata_t userdata, realm_free_userdata_func_t userdata_free);

/**
 * Registers a new email identity with the username/password provider and send confirmation email.
 * @param app ptr to realm_app
 * @param email identity email
 * @param password associated to the identity
 * @param callback invoked once operation has completed
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_email_password_provider_client_register_email(realm_app_t* app, const char* email,
                                                                     realm_string_t password,
                                                                     realm_app_void_completion_func_t callback,
                                                                     realm_userdata_t userdata,
                                                                     realm_free_userdata_func_t userdata_free);

/**
 * Confirms an email identity with the username/password provider.
 * @param app ptr to realm_app
 * @param token string emailed
 * @param token_id string emailed
 * @param callback invoked once operation has completed
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_email_password_provider_client_confirm_user(realm_app_t* app, const char* token,
                                                                   const char* token_id,
                                                                   realm_app_void_completion_func_t callback,
                                                                   realm_userdata_t userdata,
                                                                   realm_free_userdata_func_t userdata_free);

/**
 * Re-sends a confirmation email to a user that has registered but not yet confirmed their email address.
 * @param app ptr to realm_app
 * @param email to use
 * @param callback invoked once operation has completed
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_email_password_provider_client_resend_confirmation_email(
    realm_app_t* app, const char* email, realm_app_void_completion_func_t callback, realm_userdata_t userdata,
    realm_free_userdata_func_t userdata_free);

/**
 * Send reset password to the email specified in the parameter passed to the function.
 * @param app ptr to realm_app
 * @param email where to send the reset instructions
 * @param callback invoked once operation has completed
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_email_password_provider_client_send_reset_password_email(
    realm_app_t* app, const char* email, realm_app_void_completion_func_t callback, realm_userdata_t userdata,
    realm_free_userdata_func_t userdata_free);

/**
 * Retries the custom confirmation function on a user for a given email.
 * @param app ptr to realm_app
 * @param email email for the user
 * @param callback invoked once operation has completed
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_email_password_provider_client_retry_custom_confirmation(
    realm_app_t* app, const char* email, realm_app_void_completion_func_t callback, realm_userdata_t userdata,
    realm_free_userdata_func_t userdata_free);

/**
 * Resets the password of an email identity using the password reset token emailed to a user.
 * @param app ptr to realm_app
 * @param password new password to set
 * @param token ptr to token string emailed to the user
 * @param token_id ptr to token_id emailed to the user
 * @return True if no error has been recorded, False otherwise
 */
RLM_API bool realm_app_email_password_provider_client_reset_password(realm_app_t* app, realm_string_t password,
                                                                     const char* token, const char* token_id,
                                                                     realm_app_void_completion_func_t callback,
                                                                     realm_userdata_t userdata,
                                                                     realm_free_userdata_func_t userdata_free);

/**
 * Run the Email/Password Authentication provider's password reset function.
 *
 * @param serialized_ejson_payload The arguments array to invoke the function with,
 *                                 serialized as an Extended JSON string.
 * @return true, if no error occurred.
 */
RLM_API bool realm_app_email_password_provider_client_call_reset_password_function(
    realm_app_t*, const char* email, realm_string_t password, const char* serialized_ejson_payload,
    realm_app_void_completion_func_t callback, realm_userdata_t userdata, realm_free_userdata_func_t userdata_free);

/**
 * Creates a user API key that can be used to authenticate as the current user.
 * @return True if no error was recorded. False otherwise
 */
RLM_API bool realm_app_user_apikey_provider_client_create_apikey(const realm_app_t*, const realm_user_t*,
                                                                 const char* name,
                                                                 realm_return_apikey_func_t callback,
                                                                 realm_userdata_t userdata,
                                                                 realm_free_userdata_func_t userdata_free);

/**
 * Fetches a user API key associated with the current user.
 * @return True if no error was recorded. False otherwise
 */
RLM_API bool realm_app_user_apikey_provider_client_fetch_apikey(const realm_app_t*, const realm_user_t*,
                                                                realm_object_id_t id,
                                                                realm_return_apikey_func_t callback,
                                                                realm_userdata_t userdata,
                                                                realm_free_userdata_func_t userdata_free);

/**
 * Fetches the user API keys associated with the current user.
 * @return True if no error was recorded. False otherwise
 */
RLM_API bool realm_app_user_apikey_provider_client_fetch_apikeys(const realm_app_t*, const realm_user_t*,
                                                                 realm_return_apikey_list_func_t callback,
                                                                 realm_userdata_t userdata,
                                                                 realm_free_userdata_func_t userdata_free);

/**
 * Deletes a user API key associated with the current user.
 * @return True if no error was recorded. False otherwise
 */
RLM_API bool realm_app_user_apikey_provider_client_delete_apikey(const realm_app_t*, const realm_user_t*,
                                                                 realm_object_id_t id,
                                                                 realm_app_void_completion_func_t callback,
                                                                 realm_userdata_t userdata,
                                                                 realm_free_userdata_func_t userdata_free);

/**
 * Enables a user API key associated with the current user.
 * @return True if no error was recorded. False otherwise
 */
RLM_API bool realm_app_user_apikey_provider_client_enable_apikey(const realm_app_t*, const realm_user_t*,
                                                                 realm_object_id_t id,
                                                                 realm_app_void_completion_func_t callback,
                                                                 realm_userdata_t userdata,
                                                                 realm_free_userdata_func_t userdata_free);

/**
 * Disables a user API key associated with the current user.
 * @return True if no error was recorded. False otherwise
 */
RLM_API bool realm_app_user_apikey_provider_client_disable_apikey(const realm_app_t*, const realm_user_t*,
                                                                  realm_object_id_t id,
                                                                  realm_app_void_completion_func_t callback,
                                                                  realm_userdata_t userdata,
                                                                  realm_free_userdata_func_t userdata_free);

/**
 * Register a device for push notifications.
 * @return True if no error was recorded. False otherwise
 */
RLM_API bool realm_app_push_notification_client_register_device(
    const realm_app_t*, const realm_user_t*, const char* service_name, const char* registration_token,
    realm_app_void_completion_func_t, realm_userdata_t userdata, realm_free_userdata_func_t userdata_free);

/**
 * Deregister a device for push notificatons
 * @return True if no error was recorded. False otherwise
 */
RLM_API bool realm_app_push_notification_client_deregister_device(const realm_app_t*, const realm_user_t*,
                                                                  const char* service_name,
                                                                  realm_app_void_completion_func_t,
                                                                  realm_userdata_t userdata,
                                                                  realm_free_userdata_func_t userdata_free);

/**
 * Run a named MongoDB Realm function.
 *
 * @param serialized_ejson_args The arguments array to invoke the function with,
 *                        serialized as an Extended JSON string.
 * @param service_name The name of the remote service whose system function to call. Can be null,
 *                        in which case the called function is expected to be a user function.
 * @return true, if no error occurred.
 */
RLM_API bool realm_app_call_function(const realm_app_t*, const realm_user_t*, const char* function_name,
                                     const char* serialized_ejson_args, const char* service_name,
                                     realm_return_string_func_t callback, realm_userdata_t userdata,
                                     realm_free_userdata_func_t userdata_free);

/**
 * Instruct this app's sync client to immediately reconnect.
 * Useful when the device has been offline and then receives a network reachability update.
 *
 * The sync client will always attempt to reconnect eventually, this is just a hint.
 */
RLM_API void realm_app_sync_client_reconnect(realm_app_t*) RLM_API_NOEXCEPT;

/**
 * Get whether there are any active sync sessions for this app.
 */
RLM_API bool realm_app_sync_client_has_sessions(const realm_app_t*) RLM_API_NOEXCEPT;

/**
 * Wait until the sync client has terminated all sessions and released all realm files
 * it had open.
 *
 * WARNING: this is a blocking wait.
 */
RLM_API void realm_app_sync_client_wait_for_sessions_to_terminate(realm_app_t*) RLM_API_NOEXCEPT;

/**
 * Get the default realm file path based on the user and partition value in the config.
 *
 * @param custom_filename custom name for the realm file itself. Can be null,
 *                        in which case a default name based on the config will be used.
 *
 * Return value must be manually released with realm_free().
 */
RLM_API char* realm_app_sync_client_get_default_file_path_for_realm(const realm_sync_config_t*,
                                                                    const char* custom_filename);

/**
 * Get the list of identities of this @a user.
 *
 * @param out_identities A pointer to an array of `realm_user_identity_t`, which
 *                       will be populated with the list of identities of this user.
 *                       Array may be NULL, in this case no data will be copied and `out_n` set if not NULL.
 * @param capacity The maximum number of elements `out_identities` can hold.
 * @param out_n The actual number of entries written to `out_identities`. May be NULL.
 * @return true, if no errors occurred.
 */
RLM_API bool realm_user_get_all_identities(const realm_user_t* user, realm_user_identity_t* out_identities,
                                           size_t capacity, size_t* out_n);

// returned pointer must be manually released with realm_free()
RLM_API char* realm_user_get_device_id(const realm_user_t*) RLM_API_NOEXCEPT;

/**
 * Log out the user and mark it as logged out.
 *
 * Any active sync sessions associated with this user will be stopped.
 *
 * @return true, if no errors occurred.
 */
RLM_API bool realm_user_log_out(realm_user_t*);

/**
 * Get the custom user data from the user's access token.
 *
 * Returned value must be manually released with realm_free().
 *
 * @return An Extended JSON document serialized as string,
 *         or null if token doesn't have any custom data.
 */
RLM_API char* realm_user_get_custom_data(const realm_user_t*) RLM_API_NOEXCEPT;

/**
 * Get the user profile associated with this user.
 *
 * Returned value must be manually released with realm_free().
 *
 * @return An Extended JSON document serialized as string,
 *         or null if an error occurred.
 */
RLM_API char* realm_user_get_profile_data(const realm_user_t*);

/**
 * Return the identiy for the user passed as argument
 * @param user ptr to the user for which the identiy has to be retrieved
 * @return a ptr to the identity string. This must be manually released with realm_free().
 */
RLM_API char* realm_user_get_identity(const realm_user_t* user) RLM_API_NOEXCEPT;

/**
 * Retrieve the state for the user passed as argument
 * @param user ptr to the user for which the state has to be retrieved
 * @return realm_user_state_e value
 */
RLM_API realm_user_state_e realm_user_get_state(const realm_user_t* user) RLM_API_NOEXCEPT;

RLM_API bool realm_user_is_logged_in(const realm_user_t*) RLM_API_NOEXCEPT;

/**
 * Return the access token associated with the user.
 * @return a string that rapresents the access token
 */
RLM_API char* realm_user_get_access_token(const realm_user_t*);

/**
 * Return the refresh token associated with the user.
 * @return a string that represents the refresh token
 */
RLM_API char* realm_user_get_refresh_token(const realm_user_t*);

typedef struct realm_app_user_subscription_token realm_app_user_subscription_token_t;
typedef void (*realm_sync_on_user_state_changed_t)(realm_userdata_t userdata, realm_user_state_e s);
/**
 * @return a notification token object. Dispose it to stop receiving notifications.
 */
RLM_API realm_app_user_subscription_token_t*
realm_sync_user_on_state_change_register_callback(realm_user_t*, realm_sync_on_user_state_changed_t,
                                                  realm_userdata_t userdata,
                                                  realm_free_userdata_func_t userdata_free);

/**
 * In case manual reset is needed, run this function in order to reset sync client files.
 * The sync_path is going to passed into realm_sync_error_handler_func_t, if manual reset is needed.
 * This function is supposed to be called inside realm_sync_error_handler_func_t callback, if sync client reset is
 * needed
 * @param realm_app ptr to realm app.
 * @param sync_path path where the sync files are.
 * @param did_run ptr to bool, which will be set to true if operation was successful
 * @return true if operation was successful
 */
RLM_API bool realm_sync_immediately_run_file_actions(realm_app_t* realm_app, const char* sync_path,
                                                     bool* did_run) RLM_API_NOEXCEPT;

/**
 * Return the realm app for the user passed as parameter.
 * @return a ptr to the app for the user.
 */
RLM_API realm_app_t* realm_user_get_app(const realm_user_t*) RLM_API_NOEXCEPT;

#endif // REALM_APP_SERVICES

/* Sync */
typedef enum realm_sync_client_reconnect_mode {
    RLM_SYNC_CLIENT_RECONNECT_MODE_NORMAL,
    RLM_SYNC_CLIENT_RECONNECT_MODE_TESTING,
} realm_sync_client_reconnect_mode_e;

typedef enum realm_sync_session_resync_mode {
    RLM_SYNC_SESSION_RESYNC_MODE_MANUAL,
    RLM_SYNC_SESSION_RESYNC_MODE_DISCARD_LOCAL,
    RLM_SYNC_SESSION_RESYNC_MODE_RECOVER,
    RLM_SYNC_SESSION_RESYNC_MODE_RECOVER_OR_DISCARD,
} realm_sync_session_resync_mode_e;

typedef enum realm_sync_session_stop_policy {
    RLM_SYNC_SESSION_STOP_POLICY_IMMEDIATELY,
    RLM_SYNC_SESSION_STOP_POLICY_LIVE_INDEFINITELY,
    RLM_SYNC_SESSION_STOP_POLICY_AFTER_CHANGES_UPLOADED,
} realm_sync_session_stop_policy_e;

typedef enum realm_sync_session_state {
    RLM_SYNC_SESSION_STATE_ACTIVE,
    RLM_SYNC_SESSION_STATE_DYING,
    RLM_SYNC_SESSION_STATE_INACTIVE,
    RLM_SYNC_SESSION_STATE_WAITING_FOR_ACCESS_TOKEN,
    RLM_SYNC_SESSION_STATE_PAUSED,
} realm_sync_session_state_e;

typedef enum realm_sync_connection_state {
    RLM_SYNC_CONNECTION_STATE_DISCONNECTED,
    RLM_SYNC_CONNECTION_STATE_CONNECTING,
    RLM_SYNC_CONNECTION_STATE_CONNECTED,
} realm_sync_connection_state_e;

typedef enum realm_sync_progress_direction {
    RLM_SYNC_PROGRESS_DIRECTION_UPLOAD,
    RLM_SYNC_PROGRESS_DIRECTION_DOWNLOAD,
} realm_sync_progress_direction_e;

typedef enum realm_sync_error_action {
    RLM_SYNC_ERROR_ACTION_NO_ACTION,
    RLM_SYNC_ERROR_ACTION_PROTOCOL_VIOLATION,
    RLM_SYNC_ERROR_ACTION_APPLICATION_BUG,
    RLM_SYNC_ERROR_ACTION_WARNING,
    RLM_SYNC_ERROR_ACTION_TRANSIENT,
    RLM_SYNC_ERROR_ACTION_DELETE_REALM,
    RLM_SYNC_ERROR_ACTION_CLIENT_RESET,
    RLM_SYNC_ERROR_ACTION_CLIENT_RESET_NO_RECOVERY,
    RLM_SYNC_ERROR_ACTION_MIGRATE_TO_FLX,
    RLM_SYNC_ERROR_ACTION_REVERT_TO_PBS,
} realm_sync_error_action_e;

typedef enum realm_sync_file_action {
    RLM_SYNC_FILE_ACTION_DELETE_REALM,
    RLM_SYNC_FILE_ACTION_BACK_UP_THEN_DELETE_REALM,
} realm_sync_file_action_e;


typedef struct realm_sync_session realm_sync_session_t;
typedef struct realm_async_open_task realm_async_open_task_t;
typedef struct realm_sync_manager realm_sync_manager_t;

typedef struct realm_sync_error_user_info {
    const char* key;
    const char* value;
} realm_sync_error_user_info_t;

typedef struct realm_sync_error_compensating_write_info {
    const char* reason;
    const char* object_name;
    realm_value_t primary_key;
} realm_sync_error_compensating_write_info_t;

// The following interface allows C-API users to
// bring their own users. This API shouldn't be mixed
// with core's own implementation of User so it is
// only defined with app services are compiled out
#if !REALM_APP_SERVICES
/**
 * Generic completion callback for asynchronous Realm User operations.
 * @param userdata This must be the faithfully forwarded data parameter that was provided along with this callback.
 * @param error Pointer to an error object if the operation failed, otherwise null if it completed successfully.
 */
typedef void (*realm_user_void_completion_func_t)(realm_userdata_t userdata, const realm_app_error_t* error);


typedef const char* (*realm_user_get_access_token_cb_t)(realm_userdata_t userdata);
typedef const char* (*realm_user_get_refresh_token_cb_t)(realm_userdata_t userdata);
typedef realm_user_state_e (*realm_user_state_cb_t)(realm_userdata_t userdata);
typedef bool (*realm_user_access_token_refresh_required_cb_t)(realm_userdata_t userdata);
typedef realm_sync_manager_t* (*realm_user_get_sync_manager_cb_t)(realm_userdata_t userdata);
typedef void (*realm_user_request_log_out_cb_t)(realm_userdata_t userdata);
typedef void (*realm_user_request_refresh_location_cb_t)(realm_userdata_t userdata,
                                                         realm_user_void_completion_func_t cb,
                                                         realm_userdata_t cb_data);
typedef void (*realm_user_request_access_token_cb_t)(realm_userdata_t userdata, realm_user_void_completion_func_t cb,
                                                     realm_userdata_t cb_data);
typedef void (*realm_user_track_realm_cb_t)(realm_userdata_t userdata, const char* path);
typedef const char* (*realm_user_create_file_action_cb_t)(realm_userdata_t userdata, realm_sync_file_action_e action,
                                                          const char* original_path,
                                                          const char* requested_recovery_dir);
typedef struct realm_sync_user_create_config {
    realm_userdata_t userdata;
    realm_free_userdata_func_t free_func;
    const char* app_id;
    const char* user_id;
    realm_user_get_access_token_cb_t access_token_cb;
    realm_user_get_refresh_token_cb_t refresh_token_cb;
    realm_user_state_cb_t state_cb;
    realm_user_access_token_refresh_required_cb_t atrr_cb;
    realm_user_get_sync_manager_cb_t sync_manager_cb;
    realm_user_request_log_out_cb_t request_log_out_cb;
    realm_user_request_refresh_location_cb_t request_refresh_location_cb;
    realm_user_request_access_token_cb_t request_access_token_cb;
    realm_user_track_realm_cb_t track_realm_cb;
    realm_user_create_file_action_cb_t create_fa_cb;
} realm_sync_user_create_config_t;

/*
 * Construct a SyncUser instance that uses SDK provided
 * callbacks instead of core's User implementation. This type
 * of user should not be used with core's App implementation.
 */
RLM_API realm_user_t* realm_user_new(realm_sync_user_create_config_t config) RLM_API_NOEXCEPT;

/**
 * Create realm_sync_manager_t* instance given a valid realm sync client configuration.
 *
 * @return A non-null pointer if no error occurred.
 */
RLM_API realm_sync_manager_t* realm_sync_manager_create(const realm_sync_client_config_t*);

/**
 * See SyncManager::set_sync_route()
 */
RLM_API void realm_sync_manager_set_route(const realm_sync_manager_t* session, const char* route, bool is_verified);


#endif // !REALM_APP_SERVICES

// This type should never be returned from a function.
// It's only meant as an asynchronous callback argument.
// Pointers to this struct and its pointer members are only valid inside the scope
// of the callback they were passed to.
typedef struct realm_sync_error {
    realm_error_t status;
    const char* c_original_file_path_key;
    const char* c_recovery_file_path_key;
    bool is_fatal;
    bool is_unrecognized_by_client;
    bool is_client_reset_requested;
    realm_sync_error_action_e server_requests_action;

    realm_sync_error_user_info_t* user_info_map;
    size_t user_info_length;

    realm_sync_error_compensating_write_info_t* compensating_writes;
    size_t compensating_writes_length;
    void* user_code_error;
} realm_sync_error_t;

typedef struct realm_salted_file_ident {
    uint64_t ident;
    int64_t salt;
} realm_salted_file_ident_t;

/**
 * Callback function invoked by the sync session once it has uploaded or download
 * all available changesets. See @a realm_sync_session_wait_for_upload and
 * @a realm_sync_session_wait_for_download.
 *
 * This callback is invoked on the sync client's worker thread.
 *
 * @param error Null, if the operation completed successfully.
 */
typedef void (*realm_sync_wait_for_completion_func_t)(realm_userdata_t userdata, realm_error_t* error);
typedef void (*realm_sync_connection_state_changed_func_t)(realm_userdata_t userdata,
                                                           realm_sync_connection_state_e old_state,
                                                           realm_sync_connection_state_e new_state);
typedef void (*realm_sync_progress_func_t)(realm_userdata_t userdata, uint64_t transferred_bytes,
                                           uint64_t total_bytes, double progress_estimate);
typedef void (*realm_sync_error_handler_func_t)(realm_userdata_t userdata, realm_sync_session_t*,
                                                const realm_sync_error_t);
typedef bool (*realm_sync_ssl_verify_func_t)(realm_userdata_t userdata, const char* server_address, short server_port,
                                             const char* pem_data, size_t pem_size, int preverify_ok, int depth);
typedef bool (*realm_sync_before_client_reset_func_t)(realm_userdata_t userdata, realm_t* before_realm);
typedef bool (*realm_sync_after_client_reset_func_t)(realm_userdata_t userdata, realm_t* before_realm,
                                                     realm_thread_safe_reference_t* after_realm, bool did_recover);

typedef struct realm_flx_sync_subscription realm_flx_sync_subscription_t;
typedef struct realm_flx_sync_subscription_set realm_flx_sync_subscription_set_t;
typedef struct realm_flx_sync_mutable_subscription_set realm_flx_sync_mutable_subscription_set_t;
typedef struct realm_flx_sync_subscription_desc realm_flx_sync_subscription_desc_t;
typedef enum realm_flx_sync_subscription_set_state {
    RLM_SYNC_SUBSCRIPTION_UNCOMMITTED = 0,
    RLM_SYNC_SUBSCRIPTION_PENDING,
    RLM_SYNC_SUBSCRIPTION_BOOTSTRAPPING,
    RLM_SYNC_SUBSCRIPTION_COMPLETE,
    RLM_SYNC_SUBSCRIPTION_ERROR,
    RLM_SYNC_SUBSCRIPTION_SUPERSEDED,
    RLM_SYNC_SUBSCRIPTION_AWAITING_MARK,
} realm_flx_sync_subscription_set_state_e;
typedef void (*realm_sync_on_subscription_state_changed_t)(realm_userdata_t userdata,
                                                           realm_flx_sync_subscription_set_state_e state);


typedef struct realm_async_open_task_progress_notification_token realm_async_open_task_progress_notification_token_t;
typedef struct realm_sync_session_connection_state_notification_token
    realm_sync_session_connection_state_notification_token_t;

/**
 * Callback function invoked by the async open task once the realm is open and fully synchronized.
 *
 * This callback is invoked on the sync client's worker thread.
 *
 * @param realm Downloaded realm instance, or null if an error occurred.
 *              Move to the thread you want to use it on and
 *              thaw with @a realm_from_thread_safe_reference().
 *              Be aware that once received through this call, you own
 *              the object and must release it when used.
 * @param error Null, if the operation complete successfully.
 */

// invoked when the synchronized realm file has been downloaded
typedef void (*realm_async_open_task_completion_func_t)(realm_userdata_t userdata,
                                                        realm_thread_safe_reference_t* realm,
                                                        const realm_async_error_t* error);

// invoked once the file has been downloaded. Allows the caller to run some initial subscription before the completion
// callback runs.
typedef void (*realm_async_open_task_init_subscription_func_t)(realm_thread_safe_reference_t* realm,
                                                               realm_userdata_t userdata);
#if REALM_APP_SERVICES
// If using App Services, the realm_sync_client_config_t instance is part of the
// realm_app_config_t structure and this function returns a pointer to that
// member property. The realm_sync_client_config_t reference returned by this
// function should not be freed using realm_release.
RLM_API realm_sync_client_config_t* realm_app_config_get_sync_client_config(realm_app_config_t*) RLM_API_NOEXCEPT;
#else
// If not using App Services, the realm_app_config_t structure is not defined, and
// the real_sync_client_config_t structure returned by this function is meant to be
// used with realm_sync_manager_create() to create a separate Sync Manager instance.
// The realm_sync_client_config_t instance returned by this function will need to be
// manually freed using realm_release.
RLM_API realm_sync_client_config_t* realm_sync_client_config_new(void) RLM_API_NOEXCEPT;
#endif // REALM_APP_SERVICES

RLM_API void realm_sync_client_config_set_reconnect_mode(realm_sync_client_config_t*,
                                                         realm_sync_client_reconnect_mode_e) RLM_API_NOEXCEPT;
RLM_API void realm_sync_client_config_set_multiplex_sessions(realm_sync_client_config_t*, bool) RLM_API_NOEXCEPT;
RLM_API void realm_sync_client_config_set_user_agent_binding_info(realm_sync_client_config_t*,
                                                                  const char*) RLM_API_NOEXCEPT;
RLM_API void realm_sync_client_config_set_user_agent_application_info(realm_sync_client_config_t*,
                                                                      const char*) RLM_API_NOEXCEPT;
RLM_API void realm_sync_client_config_set_connect_timeout(realm_sync_client_config_t*, uint64_t) RLM_API_NOEXCEPT;
RLM_API void realm_sync_client_config_set_connection_linger_time(realm_sync_client_config_t*,
                                                                 uint64_t) RLM_API_NOEXCEPT;
RLM_API void realm_sync_client_config_set_ping_keepalive_period(realm_sync_client_config_t*,
                                                                uint64_t) RLM_API_NOEXCEPT;
RLM_API void realm_sync_client_config_set_pong_keepalive_timeout(realm_sync_client_config_t*,
                                                                 uint64_t) RLM_API_NOEXCEPT;
RLM_API void realm_sync_client_config_set_fast_reconnect_limit(realm_sync_client_config_t*,
                                                               uint64_t) RLM_API_NOEXCEPT;
RLM_API void realm_sync_client_config_set_resumption_delay_interval(realm_sync_client_config_t*,
                                                                    uint64_t) RLM_API_NOEXCEPT;
RLM_API void realm_sync_client_config_set_max_resumption_delay_interval(realm_sync_client_config_t*,
                                                                        uint64_t) RLM_API_NOEXCEPT;
RLM_API void realm_sync_client_config_set_resumption_delay_backoff_multiplier(realm_sync_client_config_t*,
                                                                              int) RLM_API_NOEXCEPT;
RLM_API void realm_sync_client_config_set_sync_socket(realm_sync_client_config_t*,
                                                      realm_sync_socket_t*) RLM_API_NOEXCEPT;
RLM_API void realm_sync_client_config_set_default_binding_thread_observer(
    realm_sync_client_config_t* config, realm_on_object_store_thread_callback_t on_thread_create,
    realm_on_object_store_thread_callback_t on_thread_destroy, realm_on_object_store_error_callback_t on_error,
    realm_userdata_t user_data, realm_free_userdata_func_t free_userdata);

RLM_API realm_sync_config_t* realm_sync_config_new(const realm_user_t*, const char* partition_value) RLM_API_NOEXCEPT;
RLM_API realm_sync_config_t* realm_flx_sync_config_new(const realm_user_t*) RLM_API_NOEXCEPT;
RLM_API void realm_sync_config_set_session_stop_policy(realm_sync_config_t*,
                                                       realm_sync_session_stop_policy_e) RLM_API_NOEXCEPT;
RLM_API void realm_sync_config_set_error_handler(realm_sync_config_t*, realm_sync_error_handler_func_t,
                                                 realm_userdata_t userdata,
                                                 realm_free_userdata_func_t userdata_free) RLM_API_NOEXCEPT;
/// DEPRECATED - Will be removed in a future release
RLM_API void realm_sync_config_set_client_validate_ssl(realm_sync_config_t*, bool) RLM_API_NOEXCEPT;
/// DEPRECATED - Will be removed in a future release
RLM_API void realm_sync_config_set_ssl_trust_certificate_path(realm_sync_config_t*, const char*) RLM_API_NOEXCEPT;
/// DEPRECATED - Will be removed in a future release
RLM_API void realm_sync_config_set_ssl_verify_callback(realm_sync_config_t*, realm_sync_ssl_verify_func_t,
                                                       realm_userdata_t userdata,
                                                       realm_free_userdata_func_t userdata_free) RLM_API_NOEXCEPT;
RLM_API void realm_sync_config_set_cancel_waits_on_nonfatal_error(realm_sync_config_t*, bool) RLM_API_NOEXCEPT;
/// DEPRECATED - Will be removed in a future release
RLM_API void realm_sync_config_set_authorization_header_name(realm_sync_config_t*, const char*) RLM_API_NOEXCEPT;
/// DEPRECATED - Will be removed in a future release
RLM_API void realm_sync_config_set_custom_http_header(realm_sync_config_t*, const char* name,
                                                      const char* value) RLM_API_NOEXCEPT;
RLM_API void realm_sync_config_set_recovery_directory_path(realm_sync_config_t*, const char*) RLM_API_NOEXCEPT;
RLM_API void realm_sync_config_set_resync_mode(realm_sync_config_t*,
                                               realm_sync_session_resync_mode_e) RLM_API_NOEXCEPT;
RLM_API void
realm_sync_config_set_before_client_reset_handler(realm_sync_config_t*, realm_sync_before_client_reset_func_t,
                                                  realm_userdata_t userdata,
                                                  realm_free_userdata_func_t userdata_free) RLM_API_NOEXCEPT;
RLM_API void
realm_sync_config_set_after_client_reset_handler(realm_sync_config_t*, realm_sync_after_client_reset_func_t,
                                                 realm_userdata_t userdata,
                                                 realm_free_userdata_func_t userdata_free) RLM_API_NOEXCEPT;
RLM_API void realm_sync_config_set_initial_subscription_handler(realm_sync_config_t*,
                                                                realm_async_open_task_init_subscription_func_t,
                                                                bool rerun_on_open, realm_userdata_t userdata,
                                                                realm_free_userdata_func_t userdata_free);
/**
 * Fetch subscription id for the subscription passed as argument.
 * @return realm_object_id_t for the subscription passed as argument
 */
RLM_API realm_object_id_t realm_sync_subscription_id(const realm_flx_sync_subscription_t* subscription)
    RLM_API_NOEXCEPT;

/**
 * Fetch subscription name for the subscription passed as argument.
 * @return realm_string_t which contains the name of the subscription.
 */
RLM_API realm_string_t realm_sync_subscription_name(const realm_flx_sync_subscription_t* subscription)
    RLM_API_NOEXCEPT;

/**
 * Fetch object class name for the subscription passed as argument.
 * @return a realm_string_t which contains the class name of the subscription.
 */
RLM_API realm_string_t realm_sync_subscription_object_class_name(const realm_flx_sync_subscription_t* subscription)
    RLM_API_NOEXCEPT;

/**
 * Fetch the query string associated with the subscription passed as argument.
 * @return realm_string_t which contains the query associated with the subscription.
 */
RLM_API realm_string_t realm_sync_subscription_query_string(const realm_flx_sync_subscription_t* subscription)
    RLM_API_NOEXCEPT;

/**
 * Fetch the timestamp in which the subscription was created for the subscription passed as argument.
 * @return realm_timestamp_t representing the timestamp in which the subscription for created.
 */
RLM_API realm_timestamp_t realm_sync_subscription_created_at(const realm_flx_sync_subscription_t* subscription)
    RLM_API_NOEXCEPT;

/**
 * Fetch the timestamp in which the subscription was updated for the subscription passed as argument.
 * @return realm_timestamp_t representing the timestamp in which the subscription was updated.
 */
RLM_API realm_timestamp_t realm_sync_subscription_updated_at(const realm_flx_sync_subscription_t* subscription)
    RLM_API_NOEXCEPT;

/**
 * Get latest subscription set
 * @return a non null subscription set pointer if such it exists.
 */
RLM_API realm_flx_sync_subscription_set_t* realm_sync_get_latest_subscription_set(const realm_t*);

/**
 * Get active subscription set
 * @return a non null subscription set pointer if such it exists.
 */
RLM_API realm_flx_sync_subscription_set_t* realm_sync_get_active_subscription_set(const realm_t*);

/**
 * Wait until subscripton set state is equal to the state passed as parameter.
 * This is a blocking operation.
 * @return the current subscription state
 */
RLM_API realm_flx_sync_subscription_set_state_e realm_sync_on_subscription_set_state_change_wait(
    const realm_flx_sync_subscription_set_t*, realm_flx_sync_subscription_set_state_e) RLM_API_NOEXCEPT;

/**
 * Register a handler in order to be notified when subscription set is equal to the one passed as parameter
 * This is an asynchronous operation.
 * @return true/false if the handler was registered correctly
 */
RLM_API bool realm_sync_on_subscription_set_state_change_async(
    const realm_flx_sync_subscription_set_t* subscription_set, realm_flx_sync_subscription_set_state_e notify_when,
    realm_sync_on_subscription_state_changed_t, realm_userdata_t userdata, realm_free_userdata_func_t userdata_free);

/**
 *  Retrieve version for the subscription set passed as parameter
 *  @return subscription set version if the poiter to the subscription is valid
 */
RLM_API int64_t realm_sync_subscription_set_version(const realm_flx_sync_subscription_set_t*) RLM_API_NOEXCEPT;

/**
 * Fetch current state for the subscription set passed as parameter
 *  @return the current state of the subscription_set
 */
RLM_API realm_flx_sync_subscription_set_state_e
realm_sync_subscription_set_state(const realm_flx_sync_subscription_set_t*) RLM_API_NOEXCEPT;

/**
 *  Query subscription set error string
 *  @return error string for the subscription passed as parameter
 */
RLM_API const char* realm_sync_subscription_set_error_str(const realm_flx_sync_subscription_set_t*) RLM_API_NOEXCEPT;

/**
 *  Retrieve the number of subscriptions for the subscription set passed as parameter
 *  @return the number of subscriptions
 */
RLM_API size_t realm_sync_subscription_set_size(const realm_flx_sync_subscription_set_t*) RLM_API_NOEXCEPT;

/**
 *  Access the subscription at index.
 *  @return the subscription or nullptr if the index is not valid
 */
RLM_API realm_flx_sync_subscription_t* realm_sync_subscription_at(const realm_flx_sync_subscription_set_t*,
                                                                  size_t index);
/**
 *  Find subscription associated to the query passed as parameter
 *  @return a pointer to the subscription or nullptr if not found
 */
RLM_API realm_flx_sync_subscription_t* realm_sync_find_subscription_by_query(const realm_flx_sync_subscription_set_t*,
                                                                             realm_query_t*) RLM_API_NOEXCEPT;

/**
 *  Find subscription associated to the results set  passed as parameter
 *  @return a pointer to the subscription or nullptr if not found
 */
RLM_API realm_flx_sync_subscription_t*
realm_sync_find_subscription_by_results(const realm_flx_sync_subscription_set_t*, realm_results_t*) RLM_API_NOEXCEPT;


/**
 *  Find subscription by name passed as parameter
 *  @return a pointer to the subscription or nullptr if not found
 */
RLM_API realm_flx_sync_subscription_t* realm_sync_find_subscription_by_name(const realm_flx_sync_subscription_set_t*,
                                                                            const char* name) RLM_API_NOEXCEPT;

/**
 *  Refresh subscription
 *  @return true/false if the operation was successful or not
 */
RLM_API bool realm_sync_subscription_set_refresh(realm_flx_sync_subscription_set_t*);

/**
 *  Convert a subscription into a mutable one in order to alter the subscription itself
 *  @return a pointer to a mutable subscription
 */
RLM_API realm_flx_sync_mutable_subscription_set_t*
realm_sync_make_subscription_set_mutable(realm_flx_sync_subscription_set_t*);

/**
 *  Clear the subscription set passed as parameter
 *  @return true/false if operation was successful
 */
RLM_API bool realm_sync_subscription_set_clear(realm_flx_sync_mutable_subscription_set_t*);

/**
 * Insert ot update the query contained inside a result object for the subscription set passed as parameter, if
 * successful the index where the query was inserted or updated is returned along with the info whether a new query
 * was inserted or not. It is possible to specify a name for the query inserted (optional).
 *  @return true/false if operation was successful
 */
RLM_API bool realm_sync_subscription_set_insert_or_assign_results(realm_flx_sync_mutable_subscription_set_t*,
                                                                  realm_results_t*, const char* name,
                                                                  size_t* out_index, bool* out_inserted);
/**
 * Insert ot update a query for the subscription set passed as parameter, if successful the index where the query
 * was inserted or updated is returned along with the info whether a new query was inserted or not. It is possible to
 * specify a name for the query inserted (optional).
 *  @return true/false if operation was successful
 */
RLM_API bool realm_sync_subscription_set_insert_or_assign_query(realm_flx_sync_mutable_subscription_set_t*,
                                                                realm_query_t*, const char* name, size_t* out_index,
                                                                bool* out_inserted);
/**
 *  Erase from subscription set by id. If operation completes successfully set the bool out param.
 *  @return true if no error occurred, false otherwise (use realm_get_last_error for fetching the error).
 */
RLM_API bool realm_sync_subscription_set_erase_by_id(realm_flx_sync_mutable_subscription_set_t*,
                                                     const realm_object_id_t*, bool* erased);
/**
 *  Erase from subscription set by name. If operation completes successfully set the bool out param.
 *  @return true if no error occurred, false otherwise (use realm_get_last_error for fetching the error)
 */
RLM_API bool realm_sync_subscription_set_erase_by_name(realm_flx_sync_mutable_subscription_set_t*, const char*,
                                                       bool* erased);
/**
 *  Erase from subscription set by query. If operation completes successfully set the bool out param.
 *  @return true if no error occurred, false otherwise (use realm_get_last_error for fetching the error)
 */
RLM_API bool realm_sync_subscription_set_erase_by_query(realm_flx_sync_mutable_subscription_set_t*, realm_query_t*,
                                                        bool* erased);
/**
 *  Erase from subscription set by results. If operation completes successfully set the bool out param.
 *  @return true if no error occurred, false otherwise (use realm_get_last_error for fetching the error)
 */
RLM_API bool realm_sync_subscription_set_erase_by_results(realm_flx_sync_mutable_subscription_set_t*,
                                                          realm_results_t*, bool* erased);
/**
 *  Remove all subscriptions for a given class type. If operation completes successfully set the bool out param.
 *  @return true if no error occurred, false otherwise (use realm_get_last_error for fetching the error).
 */
RLM_API bool realm_sync_subscription_set_erase_by_class_name(realm_flx_sync_mutable_subscription_set_t*, const char*,
                                                             bool* erased);
/**
 *  Commit the subscription_set passed as parameter (in order that all the changes made will take effect)
 *  @return pointer to a valid immutable subscription if commit was successful
 */
RLM_API realm_flx_sync_subscription_set_t*
realm_sync_subscription_set_commit(realm_flx_sync_mutable_subscription_set_t*);

/**
 * Create a task that will open a realm with the specific configuration
 * and also download all changes from the sync server.
 *
 * Use @a realm_async_open_task_start() to start the download process.
 */
RLM_API realm_async_open_task_t* realm_open_synchronized(realm_config_t*) RLM_API_NOEXCEPT;
RLM_API void realm_async_open_task_start(realm_async_open_task_t*, realm_async_open_task_completion_func_t,
                                         realm_userdata_t userdata,
                                         realm_free_userdata_func_t userdata_free) RLM_API_NOEXCEPT;
RLM_API void realm_async_open_task_cancel(realm_async_open_task_t*) RLM_API_NOEXCEPT;
RLM_API realm_async_open_task_progress_notification_token_t*
realm_async_open_task_register_download_progress_notifier(realm_async_open_task_t*, realm_sync_progress_func_t,
                                                          realm_userdata_t userdata,
                                                          realm_free_userdata_func_t userdata_free) RLM_API_NOEXCEPT;
/**
 * Get the sync session for a specific realm.
 *
 * This function will not fail if the realm wasn't open with a sync configuration in place,
 * but just return NULL;
 *
 * @return A non-null pointer if a session exists.
 */
RLM_API realm_sync_session_t* realm_sync_session_get(const realm_t*) RLM_API_NOEXCEPT;

/**
 * Fetch state for the session passed as parameter
 * @param session ptr to the sync session to retrieve the state for
 * @return realm_sync_session_state_e value
 */
RLM_API realm_sync_session_state_e realm_sync_session_get_state(const realm_sync_session_t* session) RLM_API_NOEXCEPT;

/**
 * Fetch connection state for the session passed as parameter
 * @param session ptr to the sync session to retrieve the state for
 * @return realm_sync_connection_state_e value
 */
RLM_API realm_sync_connection_state_e realm_sync_session_get_connection_state(const realm_sync_session_t* session)
    RLM_API_NOEXCEPT;

/**
 * Fetch user for the session passed as parameter
 * @param session ptr to the sync session to retrieve the user for
 * @return ptr to realm_user_t
 */
RLM_API realm_user_t* realm_sync_session_get_user(const realm_sync_session_t* session) RLM_API_NOEXCEPT;

/**
 * Fetch partition value for the session passed as parameter
 * @param session ptr to the sync session to retrieve the partition value for
 * @return a string containing the partition value
 */
RLM_API const char* realm_sync_session_get_partition_value(const realm_sync_session_t* session) RLM_API_NOEXCEPT;

/**
 * Get the filesystem path of the realm file backing this session.
 */
RLM_API const char* realm_sync_session_get_file_path(const realm_sync_session_t*) RLM_API_NOEXCEPT;

/**
 * Ask the session to pause synchronization.
 *
 * No-op if the session is already inactive.
 */
RLM_API void realm_sync_session_pause(realm_sync_session_t*) RLM_API_NOEXCEPT;

/**
 * Ask the session to resume synchronization.
 *
 * No-op if the session is already active.
 */
RLM_API void realm_sync_session_resume(realm_sync_session_t*) RLM_API_NOEXCEPT;

/**
 * Gets the file ident/salt currently assigned to the realm by sync. Callers should supply a pointer token
 * a realm_salted_file_ident_t for this function to fill out.
 */
RLM_API void realm_sync_session_get_file_ident(realm_sync_session_t*,
                                               realm_salted_file_ident_t* out) RLM_API_NOEXCEPT;


/**
 * Register a callback that will be invoked every time the session's connection state changes.
 *
 * @return a notification token object. Dispose it to stop receiving notifications.
 */
RLM_API realm_sync_session_connection_state_notification_token_t*
realm_sync_session_register_connection_state_change_callback(
    realm_sync_session_t*, realm_sync_connection_state_changed_func_t, realm_userdata_t userdata,
    realm_free_userdata_func_t userdata_free) RLM_API_NOEXCEPT;

/**
 * Register a callback that will be invoked every time the session reports progress.
 *
 * @param is_streaming If true, then the notifier will be called forever, and will
 *                     always contain the most up-to-date number of downloadable or uploadable bytes.
 *                     Otherwise, the number of downloaded or uploaded bytes will always be reported
 *                     relative to the number of downloadable or uploadable bytes at the point in time
 *                     when the notifier was registered.
 * @return a notification token object. Dispose it to stop receiving notifications.
 */
RLM_API realm_sync_session_connection_state_notification_token_t* realm_sync_session_register_progress_notifier(
    realm_sync_session_t*, realm_sync_progress_func_t, realm_sync_progress_direction_e, bool is_streaming,
    realm_userdata_t userdata, realm_free_userdata_func_t userdata_free) RLM_API_NOEXCEPT;


/**
 * Register a callback that will be invoked when all pending downloads have completed.
 */
RLM_API void
realm_sync_session_wait_for_download_completion(realm_sync_session_t*, realm_sync_wait_for_completion_func_t,
                                                realm_userdata_t userdata,
                                                realm_free_userdata_func_t userdata_free) RLM_API_NOEXCEPT;

/**
 * Register a callback that will be invoked when all pending uploads have completed.
 */
RLM_API void realm_sync_session_wait_for_upload_completion(realm_sync_session_t*,
                                                           realm_sync_wait_for_completion_func_t,
                                                           realm_userdata_t userdata,
                                                           realm_free_userdata_func_t userdata_free) RLM_API_NOEXCEPT;

/**
 * Wrapper for SyncSession::OnlyForTesting::handle_error. This routine should be used only for testing.
 * @param session ptr to a valid sync session
 * @param error_code realm_errno_e representing the error to simulate
 * @param error_str error message to be included with Status
 * @param is_fatal boolean to signal if the error is fatal or not
 */
RLM_API void realm_sync_session_handle_error_for_testing(const realm_sync_session_t* session,
                                                         realm_errno_e error_code, const char* error_str,
                                                         bool is_fatal);

/**
 * In case of exception thrown in user code callbacks, this api will allow the sdk to store the user code exception
 * and retrieve a it later via realm_get_last_error.
 * Most importantly the SDK is responsible to handle the memory pointed by user_code_error.
 * @param usercode_error pointer representing whatever object the SDK treats as exception/error.
 */
RLM_API void realm_register_user_code_callback_error(realm_userdata_t usercode_error) RLM_API_NOEXCEPT;


#if REALM_APP_SERVICES

typedef struct realm_mongodb_collection realm_mongodb_collection_t;

typedef struct realm_mongodb_find_options {
    realm_string_t projection_bson;
    realm_string_t sort_bson;
    int64_t limit;
} realm_mongodb_find_options_t;

typedef struct realm_mongodb_find_one_and_modify_options {
    realm_string_t projection_bson;
    realm_string_t sort_bson;
    bool upsert;
    bool return_new_document;
} realm_mongodb_find_one_and_modify_options_t;

typedef void (*realm_mongodb_callback_t)(realm_userdata_t userdata, realm_string_t bson,
                                         realm_app_error_t* app_error);

/**
 *  Get mongo db collection from realm mongo db client
 *  @param user ptr to the sync realm user of which we want to retrieve the remote collection for
 *  @param service name of the service where the collection will be found
 *  @param database name of the database where the collection will be found
 *  @param collection name of the collection to fetch
 *  @return a ptr to a valid mongodb collection if such collection exists, nullptr otherwise
 */
RLM_API realm_mongodb_collection_t* realm_mongo_collection_get(realm_user_t* user, const char* service,
                                                               const char* database, const char* collection);

/**
 *  Implement find for mongodb collection
 *  @param collection ptr to the collection to fetch from
 *  @param filter_ejson extended json string serialization representing the filter to apply to this operation
 *  @param options set of possible options to apply to this operation
 *  @param data user data to pass down to this function
 *  @param delete_data deleter for user data
 *  @param callback to invoke with the result
 *  @return True if completes successfully, False otherwise
 */
RLM_API bool realm_mongo_collection_find(realm_mongodb_collection_t* collection, realm_string_t filter_ejson,
                                         const realm_mongodb_find_options_t* options, realm_userdata_t data,
                                         realm_free_userdata_func_t delete_data, realm_mongodb_callback_t callback);

/**
 *  Implement find_one for mongodb collection
 *  @param collection ptr to the collection to fetch from
 *  @param filter_ejson extended json string serialization representing the filter to apply to this operation
 *  @param options set of possible options to apply to this operation
 *  @param data user data to pass down to this function
 *  @param delete_data deleter for user data
 *  @param callback to invoke with the result
 *  @return True if completes successfully, False otherwise
 */
RLM_API bool realm_mongo_collection_find_one(realm_mongodb_collection_t* collection, realm_string_t filter_ejson,
                                             const realm_mongodb_find_options_t* options, realm_userdata_t data,
                                             realm_free_userdata_func_t delete_data,
                                             realm_mongodb_callback_t callback);

/**
 *  Implement aggregate for mongodb collection
 *  @param collection ptr to the collection to fetch from
 *  @param filter_ejson extended json string serialization representing the filter to apply to this operation
 *  @param data user data to pass down to this function
 *  @param delete_data deleter for user data
 *  @param callback to invoke with the result
 *  @return True if completes successfully, False otherwise
 */
RLM_API bool realm_mongo_collection_aggregate(realm_mongodb_collection_t* collection, realm_string_t filter_ejson,
                                              realm_userdata_t data, realm_free_userdata_func_t delete_data,
                                              realm_mongodb_callback_t callback);

/**
 *  Implement count for mongodb collection
 *  @param collection ptr to the collection to fetch from
 *  @param filter_ejson extended json string serialization representing the filter to apply to this operation
 *  @param limit number of collectio
 *  @param data user data to pass down to this function
 *  @param delete_data deleter for user data
 *  @param callback to invoke with the result
 *  @return True if completes successfully, False otherwise
 */
RLM_API bool realm_mongo_collection_count(realm_mongodb_collection_t* collection, realm_string_t filter_ejson,
                                          int64_t limit, realm_userdata_t data,
                                          realm_free_userdata_func_t delete_data, realm_mongodb_callback_t callback);

/**
 *  Implement insert_one for mongodb collection
 *  @param collection name of the collection to fetch from
 *  @param filter_ejson extended json string serialization representing the filter to apply to this operation
 *  @param data user data to pass down to this function
 *  @param delete_data deleter for user data
 *  @param callback to invoke with the result
 *  @return True if completes successfully, False otherwise
 */
RLM_API bool realm_mongo_collection_insert_one(realm_mongodb_collection_t* collection, realm_string_t filter_ejson,
                                               realm_userdata_t data, realm_free_userdata_func_t delete_data,
                                               realm_mongodb_callback_t callback);

/**
 *  Implement insert_many for mongodb collection
 *  @param collection name of the collection to fetch from
 *  @param filter_ejson extended json string serialization representing the filter to apply to this operation
 *  @param data user data to pass down to this function
 *  @param delete_data deleter for user data
 *  @param callback to invoke with the result
 *  @return True if completes successfully, False otherwise
 */
RLM_API bool realm_mongo_collection_insert_many(realm_mongodb_collection_t* collection, realm_string_t filter_ejson,
                                                realm_userdata_t data, realm_free_userdata_func_t delete_data,
                                                realm_mongodb_callback_t callback);

/**
 *  Implement delete_one for mongodb collection
 *  @param collection name of the collection to fetch from
 *  @param filter_ejson extended json string serialization representing the filter to apply to this operation
 *  @param data user data to pass down to this function
 *  @param delete_data deleter for user data
 *  @param callback to invoke with the result
 *  @return True if completes successfully, False otherwise
 */
RLM_API bool realm_mongo_collection_delete_one(realm_mongodb_collection_t* collection, realm_string_t filter_ejson,
                                               realm_userdata_t data, realm_free_userdata_func_t delete_data,
                                               realm_mongodb_callback_t callback);

/**
 *  Implement delete_many for mongodb collection
 *  @param collection name of the collection to fetch from
 *  @param filter_ejson extended json string serialization representing the filter to apply to this operation
 *  @param data user data to pass down to this function
 *  @param delete_data deleter for user data
 *  @param callback to invoke with the result
 */
RLM_API bool realm_mongo_collection_delete_many(realm_mongodb_collection_t* collection, realm_string_t filter_ejson,
                                                realm_userdata_t data, realm_free_userdata_func_t delete_data,
                                                realm_mongodb_callback_t callback);

/**
 *  Implement update_one for mongodb collection
 *  @param collection name of the collection to fetch from
 *  @param filter_ejson extended json string serialization representing the filter to apply to this operation
 *  @param update_ejson extended json string serialization representing the update to apply to this operation
 *  @param upsert boolean flag to set for enable or disable upsert for the collection
 *  @param data user data to pass down to this function
 *  @param delete_data deleter for user data
 *  @param callback to invoke with the result
 *  @return True if completes successfully, False otherwise
 */
RLM_API bool realm_mongo_collection_update_one(realm_mongodb_collection_t* collection, realm_string_t filter_ejson,
                                               realm_string_t update_ejson, bool upsert, realm_userdata_t data,
                                               realm_free_userdata_func_t delete_data,
                                               realm_mongodb_callback_t callback);

/**
 *  Implement update_many for mongodb collection
 *  @param collection name of the collection to fetch from
 *  @param filter_ejson extended json string serialization representing the filter to apply to this operation
 *  @param update_ejson extended json string serialization representing the update to apply to this operation
 *  @param upsert boolean flag to set for enable or disable upsert for the collection
 *  @param data user data to pass down to this function
 *  @param delete_data deleter for user data
 *  @param callback to invoke with the result
 *  @return True if completes successfully, False otherwise
 */
RLM_API bool realm_mongo_collection_update_many(realm_mongodb_collection_t* collection, realm_string_t filter_ejson,
                                                realm_string_t update_ejson, bool upsert, realm_userdata_t data,
                                                realm_free_userdata_func_t delete_data,
                                                realm_mongodb_callback_t callback);

/**
 *  Implement find one and update for mongodb collection
 *  @param collection name of the collection to fetch from
 *  @param filter_ejson extended json string serialization representing the filter to apply to this operation
 *  @param update_ejson extended json string serialization representing the update to apply to this operation
 *  @param options set of possible options to apply to this operation
 *  @param data user data to pass down to this function
 *  @param delete_data deleter for user data
 *  @param callback to invoke with the result
 *  @return True if completes successfully, False otherwise
 */
RLM_API bool realm_mongo_collection_find_one_and_update(realm_mongodb_collection_t* collection,
                                                        realm_string_t filter_ejson, realm_string_t update_ejson,
                                                        const realm_mongodb_find_one_and_modify_options_t* options,
                                                        realm_userdata_t data, realm_free_userdata_func_t delete_data,
                                                        realm_mongodb_callback_t callback);

/**
 *  Implement find_one and replace for mongodb collection
 *  @param collection name of the collection to fetch from
 *  @param filter_ejson extended json string serialization representing the filter to apply to this operation
 *  @param replacement_ejson extended json string serialization representing the replacement object to apply to this
 * operation
 *  @param options set of possible options to apply to this operation
 *  @param data user data to pass down to this function
 *  @param delete_data deleter for user data
 *  @param callback to invoke with the result
 *  @return True if completes successfully, False otherwise
 */
RLM_API bool realm_mongo_collection_find_one_and_replace(
    realm_mongodb_collection_t* collection, realm_string_t filter_ejson, realm_string_t replacement_ejson,
    const realm_mongodb_find_one_and_modify_options_t* options, realm_userdata_t data,
    realm_free_userdata_func_t delete_data, realm_mongodb_callback_t callback);

/**
 *  Implement find_one and delete  for mongodb collection
 *  @param collection name of the collection to fetch from
 *  @param filter_ejson extended json string serialization representing the filter to apply to this operation
 *  @param options set of possible options to apply to this operation
 *  @param data user data to pass down to this function
 *  @param delete_data deleter for user data
 *  @param callback to invoke with the result
 *  @return True if completes successfully, False otherwise
 */
RLM_API bool realm_mongo_collection_find_one_and_delete(realm_mongodb_collection_t* collection,
                                                        realm_string_t filter_ejson,
                                                        const realm_mongodb_find_one_and_modify_options_t* options,
                                                        realm_userdata_t data, realm_free_userdata_func_t delete_data,
                                                        realm_mongodb_callback_t callback);

#endif // REALM_APP_SERVICES

/**
 * Creates a new sync socket instance for the Sync Client that handles the operations for a custom
 * websocket and event loop implementation.
 * @param userdata CAPI implementation specific pointer containing custom context data that is provided to
 *                 each of the provided functions.
 * @param userdata_free function that will be called when the sync socket is destroyed to delete userdata. This
 *                      is required if userdata is not null.
 * @param post_func function that will be called to post a callback handler onto the event loop - use the
 *                  realm_sync_socket_post_complete() function when the callback handler is scheduled to run.
 * @param create_timer_func function that will be called to create a new timer resource with the callback
 *                          handler that will be run when the timer expires or an erorr occurs - use the
 *                          realm_sync_socket_timer_canceled() function if the timer is canceled or the
 *                          realm_sync_socket_timer_complete() function if the timer expires or an error occurs.
 * @param cancel_timer_func function that will be called when the timer has been canceled by the sync client.
 * @param free_timer_func function that will be called when the timer resource has been destroyed by the sync client.
 * @param websocket_connect_func function that will be called when the sync client creates a websocket.
 * @param websocket_write_func function that will be called when the sync client sends data over the websocket.
 * @param websocket_free_func function that will be called when the sync client closes the websocket conneciton.
 * @return a realm_sync_socket_t pointer suitable for passing to realm_sync_client_config_set_sync_socket()
 */
RLM_API realm_sync_socket_t* realm_sync_socket_new(
    realm_userdata_t userdata, realm_free_userdata_func_t userdata_free, realm_sync_socket_post_func_t post_func,
    realm_sync_socket_create_timer_func_t create_timer_func,
    realm_sync_socket_timer_canceled_func_t cancel_timer_func, realm_sync_socket_timer_free_func_t free_timer_func,
    realm_sync_socket_connect_func_t websocket_connect_func,
    realm_sync_socket_websocket_async_write_func_t websocket_write_func,
    realm_sync_socket_websocket_free_func_t websocket_free_func);

/**
 * To be called to execute the callback handler provided to the create_timer_func when the timer is
 * complete or an error occurs while processing the timer.
 * @param timer_handler the timer callback handler that was provided when the timer was created.
 * @param result the error code for the error that occurred or RLM_ERR_SYNC_SOCKET_SUCCESS if the timer
 *               expired normally.
 * @param reason a string describing details about the error that occurred or empty string if no error.
 * NOTE: This function must be called by the event loop execution thread.
 */
RLM_API void realm_sync_socket_timer_complete(realm_sync_socket_timer_callback_t* timer_handler,
                                              realm_sync_socket_callback_result_e result, const char* reason);

/**
 * To be called to execute the callback handler provided to the create_timer_func when the timer has been
 * canceled.
 * @param timer_handler the timer callback handler that was provided when the timer was created.
 * NOTE: This function must be called by the event loop execution thread.
 */
RLM_API void realm_sync_socket_timer_canceled(realm_sync_socket_timer_callback_t* timer_handler);

/**
 * To be called to execute the callback function provided to the post_func when the event loop executes
 * that post'ed operation. The post_handler resource will automatically be destroyed during this
 * operation.
 * @param post_handler the post callback handler that was originally provided to the post_func
 * @param result the error code for the error that occurred or RLM_ERR_SYNC_SOCKET_SUCCESS if the
 *               callback handler should be executed normally.
 * @param reason a string describing details about the error that occurred or empty string if no error.
 * NOTE: This function must be called by the event loop execution thread.
 */
RLM_API void realm_sync_socket_post_complete(realm_sync_socket_post_callback_t* post_handler,
                                             realm_sync_socket_callback_result_e result, const char* reason);

/**
 * To be called to execute the callback function provided to the websocket_write_func when the write
 * operation is complete. The write_handler resource will automatically be destroyed during this
 * operation.
 * @param write_handler the write callback handler that was originally provided to the websocket_write_func
 * @param result the error code for the error that occurred or RLM_ERR_SYNC_SOCKET_SUCCESS if write completed
 *               successfully
 * @param reason a string describing details about the error that occurred or empty string if no error.
 * NOTE: This function must be called by the event loop execution thread.
 */
RLM_API void realm_sync_socket_write_complete(realm_sync_socket_write_callback_t* write_handler,
                                              realm_sync_socket_callback_result_e result, const char* reason);

/**
 * To be called when the websocket successfully connects to the server.
 * @param realm_websocket_observer the websocket observer object that was provided to the websocket_connect_func
 * @param protocol the value of the Sec-WebSocket-Protocol header in the connect response from the server.
 * NOTE: This function must be called by the event loop execution thread and should not be called
 *       after the websocket_free_func has been called to release the websocket resources.
 */
RLM_API void realm_sync_socket_websocket_connected(realm_websocket_observer_t* realm_websocket_observer,
                                                   const char* protocol);

/**
 * To be called when an error occurs - the actual error value will be provided when the websocket_closed
 * function is called. This function informs that the socket object is in an error state and no further
 * TX operations should be performed.
 * @param realm_websocket_observer the websocket observer object that was provided to the websocket_connect_func
 * NOTE: This function must be called by the event loop execution thread and should not be called
 *       after the websocket_free_func has been called to release the websocket resources.
 */
RLM_API void realm_sync_socket_websocket_error(realm_websocket_observer_t* realm_websocket_observer);

/**
 * To be called to provide the received data to the Sync Client when a write operation has completed.
 * The data buffer can be safely discarded after this function has completed.
 * @param realm_websocket_observer the websocket observer object that was provided to the websocket_connect_func
 * @param data a pointer to the buffer that contains the data received over the websocket
 * @param data_size the number of bytes in the data buffer
 * @return bool designates whether the WebSocket object should continue processing messages. The normal return
 *         value is true. False must be returned if the websocket object has been destroyed during execution of
 *         the function.
 * NOTE: This function must be called by the event loop execution thread and should not be called
 *       after the websocket_free_func has been called to release the websocket resources.
 */
RLM_API bool realm_sync_socket_websocket_message(realm_websocket_observer_t* realm_websocket_observer,
                                                 const char* data, size_t data_size);

/**
 * To be called when the websocket has been closed, either due to an error or a normal close operation.
 * @param realm_websocket_observer the websocket observer object that was provided to the websocket_connect_func
 * @param was_clean boolean value that indicates whether this is a normal close situation (true), the
 *                  close code was provided by the server via a close message (true), or if the close code was
 *                  generated by the local websocket as a result of some other error (false) (e.g. host
 *                  unreachable, etc.)
 * @param code the websocket close code (per the WebSocket spec) that describes why the websocket was closed.
 * @param reason a string describing details about the error that occurred or empty string if no error.
 * @return bool designates whether the WebSocket object has been destroyed during the execution of this
 *         function. The normal return value is True to indicate the WebSocket object is no longer valid. If
 *         False is returned, the WebSocket object will be destroyed at some point in the future.
 * NOTE: This function must be called by the event loop execution thread and should not be called
 *       after the websocket_free_func has been called to release the websocket resources.
 */
RLM_API bool realm_sync_socket_websocket_closed(realm_websocket_observer_t* realm_websocket_observer, bool was_clean,
                                                realm_web_socket_errno_e code, const char* reason);

#endif // REALM_H
