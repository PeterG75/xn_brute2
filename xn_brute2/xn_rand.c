/*  
    The MIT License (MIT)
    [OSI Approved License]

    The MIT License (MIT)

    Copyright (c) 2015 axiuno@gmail.com

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

    http://www.opensource.org/licenses/mit-license.php 
*/

#include "pch.h"
#include "mt_rand.h"
#include "xn_rand.h"

// 随机字符
void xn_rand(struct mt_s *mt, char *buffer, size_t n)
{
    const char str[] = "23456789ABCDEFGHJKMNPQRSTUVWXYZ";
    const long len = _countof(str) - 1;
    long r;

    for ( size_t i = 0; i < n - 1; ++i ) {
        r = mt_rand_range(mt, 1, len);
        buffer[i] = str[r - 1];
    }
    buffer[n - 1] = '\0';
}
