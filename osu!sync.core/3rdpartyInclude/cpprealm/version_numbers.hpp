/*************************************************************************
*
* Copyright 2023 Realm Inc.
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

#ifndef REALMCXX_VERSION_NUMBERS_HPP
#define REALMCXX_VERSION_NUMBERS_HPP

// Do not use `cmakedefine` here, as certain versions can be 0, which CMake
// interprets as being undefined.
#define REALMCXX_VERSION_MAJOR 2
#define REALMCXX_VERSION_MINOR 2
#define REALMCXX_VERSION_PATCH 0
#define REALMCXX_VERSION_STRING "2.2.0"

#endif // REALMCXX_VERSION_NUMBERS_HPP
