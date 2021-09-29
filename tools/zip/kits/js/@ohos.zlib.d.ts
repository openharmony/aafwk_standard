/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { AsyncCallback } from './basic';

declare namespace zlib {
    // Zlib library constant definition
    export enum FlushType {
        FLUSH_TYPE_NO_FLUSH = 0,
        FLUSH_TYPE_PARTIAL_FLUSH = 1,
        FLUSH_TYPE_SYNC_FLUSH = 2,
        FLUSH_TYPE_FULL_FLUSH = 3,
        FLUSH_TYPE_FINISH = 4,
        FLUSH_TYPE_BLOCK = 5,
        FLUSH_TYPE_TREES = 6
    }

    // The return code of the compression and decompression function. 
	// A negative number indicates an error and a positive number indicates a normal event
    // enum ErrorCode 
    export enum ErrorCode {
        ERROR_CODE_OK = 0,    
        ERROR_CODE_STREAM_END = 1,
        ERROR_CODE_NEED_DICT = 2,
        ERROR_CODE_ERRNO = -1,
        ERROR_CODE_STREAM_ERROR = -2,
        ERROR_CODE_DATA_ERROR = -3,
        ERROR_CODE_MEM_ERROR = -4,
        ERROR_CODE_BUF_ERROR = -5,
        ERROR_CODE_VERSION_ERROR = -6
    }
	
    // enum CompressLevel
    export enum CompressLevel {
        COMPRESS_LEVEL_NO_COMPRESSION = 0,
        COMPRESS_LEVEL_BEST_SPEED = 1,
        COMPRESS_LEVEL_BEST_COMPRESSION = 9,
        COMPRESS_LEVEL_DEFAULT_COMPRESSION = -1
    }

    // enum CompressStrategy
    export enum CompressStrategy {
        COMPRESS_STRATEGY_DEFAULT_STRATEGY = 0,
        COMPRESS_STRATEGY_FILTERED = 1,
        COMPRESS_STRATEGY_HUFFMAN_ONLY = 2,
        COMPRESS_STRATEGY_RLE = 3,
        COMPRESS_STRATEGY_FIXED = 4
    }

    // enum MemLevel
    export enum MemLevel {
        MEM_LEVEL_MIN_MEMLEVEL = 1,
        MEM_LEVEL_DEFAULT_MEMLEVEL = 8,
        MEM_LEVEL_MAX_MEMLEVEL = 9
    }

    // zip options
    interface Options {
        flush?: FlushType;       // corresponding zlib library macro
        finishFlush?: FlushType; // corresponding zlib library macro  #define Z_FINISH        4
        chunkSize?: number;      // Buffer size，>64
        level?: CompressLevel;   // Compression level. The compression level is a number from 0 to 9.
								 // 0 has the fastest compression speed (compression process), 9 has the slowest compression speed,
								 // the largest compression rate, and 0 does not compress.
								 
        memLevel?: MemLevel;     // How much memory should be allocated for the specified internal compression state. 
								 // Memlevel = 1 uses the minimum memory, but it is very slow, reducing the compression ratio; Memlevel = 9 uses the maximum memory to get the best speed. The default value is 8.
        strategy?: CompressStrategy;    // CompressStrategy
        dictionary?: ArrayBuffer; // deflate/inflate only, empty dictionary by default
    }
    
  
    // The interface for compressing the specified file. There is only one callback callback interface
    function zipFile(inFile:string, outFile:string, options: Options, callback: AsyncCallback<void>): void;
	function zipFile(inFile:string, outFile:string, options: Options): Promise<number>;

    // The interface for decompressing the specified file. There is only one callback callback interface
    function unzipFile(inFile:string, outFile:string, options: Options, callback: AsyncCallback<void>): void;
	function unzipFile(inFile:string, outFile:string, options: Options): Promise<number>;
}
