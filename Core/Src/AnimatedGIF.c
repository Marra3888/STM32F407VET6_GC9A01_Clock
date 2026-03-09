//
// GIF Animator
// written by Larry Bank
// bitbank@pobox.com
// Arduino port started 7/5/2020
// Original GIF code written 20+ years ago :)
// The goal of this code is to decode images up to 480x320
// using no more than 22K of RAM (if sent directly to an LCD display)
//
// Copyright 2020 BitBank Software, Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//===========================================================================
#include "AnimatedGIF.h"
#include <string.h>

// Макросы для STM32 (если их нет)
#ifndef memcpy_P
#define memcpy_P memcpy
#endif

// Константы для декодера
static const unsigned char cGIFBits[9] = {1,4,4,4,8,8,8,8,8};

// Внутренние функции (прототипы)
static int GIFInit(GIFIMAGE *pGIF);
static int GIFParseInfo(GIFIMAGE *pPage, int bInfoOnly);
static int GIFGetMoreData(GIFIMAGE *pPage);
static void GIFMakePels(GIFIMAGE *pPage, unsigned int code);
static int DecodeLZW(GIFIMAGE *pImage, int iOptions);
static int DecodeLZWTurbo(GIFIMAGE *pImage, int iOptions);
static int32_t readMem(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen);
static int32_t seekMem(GIFFILE *pFile, int32_t iPosition);

// ===========================================================================
// C API (Открытые функции)
// ===========================================================================

// Инициализация структуры (вместо конструктора)
void GIF_begin(GIFIMAGE *pGIF, unsigned char ucPaletteType)
{
    memset(pGIF, 0, sizeof(GIFIMAGE));
    pGIF->ucPaletteType = ucPaletteType;
    pGIF->ucDrawType = GIF_DRAW_RAW;
    pGIF->pFrameBuffer = NULL;
    pGIF->pTurboBuffer = NULL;
}

// Открытие GIF из массива в памяти (Flash или RAM)
int GIF_openRAM(GIFIMAGE *pGIF, uint8_t *pData, int iDataSize, GIF_DRAW_CALLBACK *pfnDraw)
{
    pGIF->iError = GIF_SUCCESS;
    pGIF->pfnRead = readMem;
    pGIF->pfnSeek = seekMem;
    pGIF->pfnDraw = pfnDraw;
    pGIF->pfnOpen = NULL;
    pGIF->pfnClose = NULL;
    pGIF->GIFFile.iSize = iDataSize;
    pGIF->GIFFile.pData = pData;

    return GIFInit(pGIF);
}

void GIF_close(GIFIMAGE *pGIF)
{
    // Очистка, если нужно
    if (pGIF->pTurboBuffer) {
        // free(pGIF->pTurboBuffer); // Если использовали malloc
    }
}

// Воспроизведение одного кадра
int GIF_playFrame(GIFIMAGE *pGIF, int *delayMilliseconds, void *pUser)
{
    int rc;

    if (delayMilliseconds) *delayMilliseconds = 0;

    if (pGIF->GIFFile.iPos >= pGIF->GIFFile.iSize-1) // Конец файла
    {
        (*pGIF->pfnSeek)(&pGIF->GIFFile, 0); // Перемотка в начало
    }

    if (GIFParseInfo(pGIF, 0))
    {
        pGIF->pUser = pUser;
        if (pGIF->iError == GIF_EMPTY_FRAME) return 0;

        // Декодирование
        if (pGIF->pTurboBuffer) {
            rc = DecodeLZWTurbo(pGIF, 0);
        } else {
            rc = DecodeLZW(pGIF, 0);
        }

        if (rc != 0) return 0; // Ошибка
    }
    else
    {
        return 0; // Ошибка парсинга
    }

    if (delayMilliseconds)
        *delayMilliseconds = pGIF->iFrameDelay;

    // Возвращаем 1, если есть еще кадры, или 0 если это был последний
    return (pGIF->GIFFile.iPos < pGIF->GIFFile.iSize-1);
}

// Получение информации
int GIF_getCanvasWidth(GIFIMAGE *pGIF) { return pGIF->iCanvasWidth; }
int GIF_getCanvasHeight(GIFIMAGE *pGIF) { return pGIF->iCanvasHeight; }
int GIF_getInfo(GIFIMAGE *pPage, GIFINFO *pInfo); // Реализована ниже

// ===========================================================================
// ВНУТРЕННИЕ ФУНКЦИИ (HELPER FUNCTIONS)
// ===========================================================================

static int32_t readMem(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    int32_t iBytesRead = iLen;
    if ((pFile->iSize - pFile->iPos) < iLen)
       iBytesRead = pFile->iSize - pFile->iPos;
    if (iBytesRead <= 0) return 0;

    // Используем memcpy, так как читаем из Flash (который отображен в память)
    memcpy(pBuf, &pFile->pData[pFile->iPos], iBytesRead);
    pFile->iPos += iBytesRead;
    return iBytesRead;
}

static int32_t seekMem(GIFFILE *pFile, int32_t iPosition)
{
    if (iPosition < 0) iPosition = 0;
    else if (iPosition >= pFile->iSize) iPosition = pFile->iSize-1;
    pFile->iPos = iPosition;
    return iPosition;
}

// Инициализация парсера
static int GIFInit(GIFIMAGE *pGIF)
{
    pGIF->GIFFile.iPos = 0;
    if (!GIFParseInfo(pGIF, 1)) return 0;
    (*pGIF->pfnSeek)(&pGIF->GIFFile, 0);
    if (pGIF->iCanvasWidth > MAX_WIDTH) { // MAX_WIDTH define in header
        pGIF->iError = GIF_TOO_WIDE;
        return 0;
    }
    return 1;
}

// Парсинг заголовков GIF
static int GIFParseInfo(GIFIMAGE *pPage, int bInfoOnly)
{
    int i, j, iColorTableBits;
    int iBytesRead;
    unsigned char c, *p;
    int32_t iOffset = 0;
    int32_t iStartPos = pPage->GIFFile.iPos;
    int iReadSize;

    pPage->bUseLocalPalette = 0;
    pPage->bEndOfFrame = 0;
    pPage->iFrameDelay = 0;
    pPage->iRepeatCount = -1;
    iReadSize = MAX_CHUNK_SIZE;

    if (iStartPos + iReadSize > pPage->GIFFile.iSize)
       iReadSize = (pPage->GIFFile.iSize - iStartPos - 1);

    p = pPage->ucFileBuf;
    iBytesRead = (*pPage->pfnRead)(&pPage->GIFFile, pPage->ucFileBuf, iReadSize);

    if (iBytesRead != iReadSize) { pPage->iError = GIF_EARLY_EOF; return 0; }

    if (iStartPos == 0) // Start of file
    {
        if (memcmp(p, "GIF89", 5) != 0 && memcmp(p, "GIF87", 5) != 0) {
           pPage->iError = GIF_BAD_FILE; return 0;
        }
        pPage->iCanvasWidth = pPage->iWidth = INTELSHORT(&p[6]);
        pPage->iCanvasHeight = pPage->iHeight = INTELSHORT(&p[8]);
        pPage->iBpp = ((p[10] & 0x70) >> 4) + 1;
        iColorTableBits = (p[10] & 7) + 1;
        pPage->ucBackground = p[11];
        pPage->ucGIFBits = 0;
        iOffset = 13;

        if (p[10] & 0x80) // Global color table
        {
            iBytesRead += (*pPage->pfnRead)(&pPage->GIFFile, &pPage->ucFileBuf[iBytesRead], 3*(1<<iColorTableBits));
            if (pPage->ucPaletteType == GIF_PALETTE_RGB565_LE || pPage->ucPaletteType == GIF_PALETTE_RGB565_BE) {
                for (i=0; i<(1<<iColorTableBits); i++) {
                    uint16_t usRGB565;
                    usRGB565 = ((p[iOffset] >> 3) << 11);
                    usRGB565 |= ((p[iOffset+1] >> 2) << 5);
                    usRGB565 |= (p[iOffset+2] >> 3);
                    if (pPage->ucPaletteType == GIF_PALETTE_RGB565_LE)
                        pPage->pPalette[i] = usRGB565;
                    else
                        pPage->pPalette[i] = __builtin_bswap16(usRGB565);
                    iOffset += 3;
                }
            } else {
                memcpy(pPage->pPalette, &p[iOffset], (1<<iColorTableBits) * 3);
                iOffset += (1 << iColorTableBits) * 3;
            }
        }
    }

    while (p[iOffset] != ',' && p[iOffset] != ';')
    {
        if (p[iOffset] == '!') { // Extension
            iOffset++;
            switch(p[iOffset++]) {
                case 0xf9: // Graphic extension
                    if (p[iOffset] == 4) {
                        pPage->ucGIFBits = p[iOffset+1];
                        pPage->iFrameDelay = (INTELSHORT(&p[iOffset+2]))*10;
                        if (pPage->iFrameDelay <= 1) pPage->iFrameDelay = 20;
                        if (pPage->ucGIFBits & 1) pPage->ucTransparent = p[iOffset+4];
                        iOffset += 6;
                    }
                    break;
                case 0xff: // App extension
                    c = 1;
                    while (c) {
                        c = p[iOffset++];
                        if ((iBytesRead - iOffset) < (c+32)) {
                            memmove(pPage->ucFileBuf, &pPage->ucFileBuf[iOffset], (iBytesRead-iOffset));
                            iBytesRead -= iOffset;
                            iStartPos += iOffset;
                            iOffset = 0;
                            iBytesRead += (*pPage->pfnRead)(&pPage->GIFFile, &pPage->ucFileBuf[iBytesRead], c+32);
                        }
                        if (c == 11) {
                            if (memcmp(&p[iOffset], "NETSCAPE2.0", 11) == 0) {
                                if (p[iOffset+11] == 3 && p[iOffset+12] == 1)
                                    pPage->iRepeatCount = INTELSHORT(&p[iOffset+13]);
                            }
                        }
                        iOffset += (int)c;
                    }
                    break;
                default: // Comment or Text
                    c = 1;
                    while (c) {
                        c = p[iOffset++];
                        if ((iBytesRead - iOffset) < (c+32)) {
                            memmove(pPage->ucFileBuf, &pPage->ucFileBuf[iOffset], (iBytesRead-iOffset));
                            iBytesRead -= iOffset;
                            iStartPos += iOffset;
                            iOffset = 0;
                            iBytesRead += (*pPage->pfnRead)(&pPage->GIFFile, &pPage->ucFileBuf[iBytesRead], c+32);
                        }
                        iOffset += (int)c;
                    }
                    break;
            }
        } else {
             if (pPage->GIFFile.iSize - iStartPos < 32) pPage->iError = GIF_EMPTY_FRAME;
             else pPage->iError = GIF_DECODE_ERROR;
             return 0;
        }
    }

    if (bInfoOnly) return 1;
    if (p[iOffset] == ';') { pPage->iError = GIF_EMPTY_FRAME; return 1; }
    if (p[iOffset] == ',') iOffset++;

    pPage->iX = INTELSHORT(&p[iOffset]);
    pPage->iY = INTELSHORT(&p[iOffset+2]);
    pPage->iWidth = INTELSHORT(&p[iOffset+4]);
    pPage->iHeight = INTELSHORT(&p[iOffset+6]);

    if (pPage->iWidth > pPage->iCanvasWidth || pPage->iHeight > pPage->iCanvasHeight) {
        pPage->iError = GIF_DECODE_ERROR; return 0;
    }
    iOffset += 8;

    pPage->ucMap = p[iOffset++];
    if (pPage->ucMap & 0x80) { // Local Palette
        j = (1<<((pPage->ucMap & 7)+1));
        iBytesRead += (*pPage->pfnRead)(&pPage->GIFFile, &pPage->ucFileBuf[iBytesRead], j*3);
        if (pPage->ucPaletteType == GIF_PALETTE_RGB565_LE || pPage->ucPaletteType == GIF_PALETTE_RGB565_BE) {
            for (i=0; i<j; i++) {
                uint16_t usRGB565;
                usRGB565 = ((p[iOffset] >> 3) << 11);
                usRGB565 |= ((p[iOffset+1] >> 2) << 5);
                usRGB565 |= (p[iOffset+2] >> 3);
                if (pPage->ucPaletteType == GIF_PALETTE_RGB565_LE) pPage->pLocalPalette[i] = usRGB565;
                else pPage->pLocalPalette[i] = __builtin_bswap16(usRGB565);
                iOffset += 3;
            }
        } else {
            memcpy(pPage->pLocalPalette, &p[iOffset], j * 3);
            iOffset += j*3;
        }
        pPage->bUseLocalPalette = 1;
    }
    pPage->ucCodeStart = p[iOffset++];
    pPage->iBpp = cGIFBits[pPage->ucCodeStart];
    pPage->iLZWSize = 0;
    c = 1;
    while (c && iOffset < iBytesRead) {
        c = p[iOffset++];
        if (c <= (iBytesRead - iOffset)) {
            memcpy(&pPage->ucLZW[pPage->iLZWSize], &p[iOffset], c);
            pPage->iLZWSize += c;
            iOffset += c;
        } else {
            int iPartialLen = (iBytesRead - iOffset);
            memcpy(&pPage->ucLZW[pPage->iLZWSize], &p[iOffset], iPartialLen);
            pPage->iLZWSize += iPartialLen;
            iOffset += iPartialLen;
            (*pPage->pfnRead)(&pPage->GIFFile, &pPage->ucLZW[pPage->iLZWSize], c - iPartialLen);
            pPage->iLZWSize += (c - iPartialLen);
        }
        if (c == 0) pPage->bEndOfFrame = 1;
    }
    if (iOffset < iBytesRead) {
        (*pPage->pfnSeek)(&pPage->GIFFile, iStartPos + iOffset);
    }
    return 1;
}

static int GIFGetMoreData(GIFIMAGE *pPage)
{
    int iDelta = (pPage->iLZWSize - pPage->iLZWOff);
    unsigned char c = 1;

    if (pPage->bEndOfFrame || iDelta >= (LZW_BUF_SIZE - MAX_CHUNK_SIZE) || iDelta <= 0) return 1;

    if (pPage->iLZWOff != 0) {
      for (int i=0; i<pPage->iLZWSize - pPage->iLZWOff; i++) {
         pPage->ucLZW[i] = pPage->ucLZW[i + pPage->iLZWOff];
      }
      pPage->iLZWSize -= pPage->iLZWOff;
      pPage->iLZWOff = 0;
    }
    while (c && pPage->GIFFile.iPos < pPage->GIFFile.iSize && pPage->iLZWSize < (LZW_BUF_SIZE-MAX_CHUNK_SIZE)) {
        (*pPage->pfnRead)(&pPage->GIFFile, &c, 1);
        (*pPage->pfnRead)(&pPage->GIFFile, &pPage->ucLZW[pPage->iLZWSize], c);
        pPage->iLZWSize += c;
    }
    if (c == 0) pPage->bEndOfFrame = 1;
    return (c != 0 && pPage->GIFFile.iPos < pPage->GIFFile.iSize);
}

// Макрос для извлечения кода LZW
#define GET_CODE \
    if (bitnum > (REGISTER_WIDTH - codesize)) { \
        pImage->iLZWOff += (bitnum >> 3); \
        bitnum &= 7; \
        ulBits = INTELLONG(&p[pImage->iLZWOff]); \
    } \
    code = (unsigned short) (ulBits >> bitnum); \
    code &= sMask; \
    bitnum += codesize;

static int DecodeLZWTurbo(GIFIMAGE *pImage, int iOptions) {
    return DecodeLZW(pImage, iOptions); // Используем обычный LZW, Turbo требует malloc
}

static int DecodeLZW(GIFIMAGE *pImage, int iOptions)
{
    int i, bitnum;
    unsigned short oldcode, codesize, nextcode, nextlim;
    unsigned short *giftabs, cc, eoi;
    signed short sMask;
    unsigned char c, *gifpels, *p;
    BIGUINT ulBits;
    unsigned short code;

    p = pImage->ucLZW;
    sMask = 0xffff << (pImage->ucCodeStart + 1);
    sMask = 0xffff - sMask;
    cc = (sMask >> 1) + 1;
    eoi = cc + 1;
    giftabs = pImage->usGIFTable;
    gifpels = pImage->ucGIFPixels;
    pImage->iYCount = pImage->iHeight;
    pImage->iXCount = pImage->iWidth;
    bitnum = 0;
    pImage->iLZWOff = 0;
    GIFGetMoreData(pImage);

    for (i = 0; i < cc; i++) {
        gifpels[PIXEL_FIRST + i] = gifpels[PIXEL_LAST + i] = (unsigned short) i;
        giftabs[i] = LINK_END;
    }
init_codetable:
    codesize = pImage->ucCodeStart + 1;
    sMask = 0xffff << (pImage->ucCodeStart + 1);
    sMask = 0xffff - sMask;
    nextcode = cc + 2;
    nextlim = (unsigned short) ((1 << codesize));
    memset(&giftabs[cc], LINK_UNUSED, sizeof(pImage->usGIFTable) - sizeof(giftabs[0])*cc);

    ulBits = INTELLONG(&p[pImage->iLZWOff]);
    GET_CODE
    if (code == cc) { GET_CODE }
    c = oldcode = code;
    GIFMakePels(pImage, code);

    while (code != eoi && pImage->iYCount > 0) {
        GET_CODE
        if (code == cc) goto init_codetable;
        if (code != eoi) {
            if (nextcode < nextlim) {
                giftabs[nextcode] = oldcode;
                gifpels[PIXEL_FIRST + nextcode] = c;
                gifpels[PIXEL_LAST + nextcode] = c = gifpels[PIXEL_FIRST + code];
            }
            nextcode++;
            if (nextcode >= nextlim && codesize < MAX_CODE_SIZE) {
                codesize++;
                nextlim <<= 1;
                sMask = nextlim - 1;
            }
            GIFMakePels(pImage, code);
            oldcode = code;
        }
    }
    return 0;
}

static void GIFMakePels(GIFIMAGE *pPage, unsigned int code)
{
    int iPixCount;
    unsigned short *giftabs;
    unsigned char *buf, *s, *pEnd, *gifpels;

    pEnd = pPage->ucFileBuf;
    s = pEnd + FILE_BUF_SIZE;
    buf = pPage->ucLineBuf + (pPage->iWidth - pPage->iXCount);
    giftabs = pPage->usGIFTable;
    gifpels = &pPage->ucGIFPixels[PIXEL_LAST];

    while (code < LINK_UNUSED) {
        *(--s) = gifpels[code];
        code = giftabs[code];
    }
    iPixCount = (int)(intptr_t)(pEnd + FILE_BUF_SIZE - s);

    while (iPixCount && pPage->iYCount > 0) {
        if (pPage->iXCount > iPixCount) {
            memcpy(buf, s, iPixCount);
            pPage->iXCount -= iPixCount;
            if (pPage->iLZWOff >= LZW_HIGHWATER) GIFGetMoreData(pPage);
            return;
        }
        else {
            GIFDRAW gd;
            memcpy(buf, s, pPage->iXCount);
            s += pPage->iXCount;
            iPixCount -= pPage->iXCount;
            pPage->iXCount = pPage->iWidth;

            // Prepare GIFDRAW
            gd.iX = pPage->iX;
            gd.iY = pPage->iY;
            gd.iWidth = pPage->iWidth;
            gd.iHeight = pPage->iHeight;
            gd.pPixels = pPage->ucLineBuf;
            gd.pPalette = (pPage->bUseLocalPalette) ? pPage->pLocalPalette : pPage->pPalette;
            gd.ucHasTransparency = pPage->ucGIFBits & 1;
            gd.ucTransparent = pPage->ucTransparent;
            gd.y = pPage->iHeight - pPage->iYCount;

            if (pPage->pfnDraw) (*pPage->pfnDraw)(&gd);

            pPage->iYCount--;
            buf = pPage->ucLineBuf;
            if (pPage->iLZWOff >= LZW_HIGHWATER) GIFGetMoreData(pPage);
        }
    }
    if (pPage->iLZWOff >= LZW_HIGHWATER) GIFGetMoreData(pPage);
}
