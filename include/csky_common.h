/*
 * csky demo common header file
 *
 * Copyright 2017 C-SKY Microsystems Co., Ltd.
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
 */

#ifndef CSKY_COMMON_H
#define CSKY_COMMON_H

#if defined(__cplusplus)
extern "C" {
#endif

#define LOG_NUMBER    5		/* total number */

#define LOG_EMERG     0		/* System is unusable */
#define LOG_ERR       1		/* Error conditions */
#define LOG_WARNING   2		/* Warning conditions */
#define LOG_INFO      3		/* Informational message */
#define LOG_DEBUG     4		/* Debug-level message */

#ifndef CONFIG_LOG_LEVEL
#define CONFIG_LOG_LEVEL  3
#endif

#if CONFIG_LOG_LEVEL >= LOG_EMERG
#define LOG_M(format, ...)     printf("[M]"__FILE__":%d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define LOG_M(format, ...)

#endif

#if CONFIG_LOG_LEVEL >= LOG_ERR
#define LOG_E(format, ...)     printf("[E]"__FILE__":%d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define LOG_E(format, ...)
#endif

#if CONFIG_LOG_LEVEL >= LOG_WARNING
#define LOG_W(format, ...)     printf("[W]"__FILE__":%d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define LOG_W(format, ...)
#endif

#if CONFIG_LOG_LEVEL >= LOG_INFO
#define LOG_I(format, ...)     printf("[I]"__FILE__":%d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define LOG_I(format, ...)
#endif

#if CONFIG_LOG_LEVEL >= LOG_DEBUG
#define LOG_D(format, ...)     printf("[D]"__FILE__":%d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define LOG_D(format, ...)
#endif

#if defined(__cplusplus)
}
#endif
#endif				/* CSKY_COMMON_H */
