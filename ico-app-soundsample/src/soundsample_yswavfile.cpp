/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the 
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   Sound Sample APP
 *          Test use with which sound is sounded
 *
 * @date    Mar-04-2013
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "soundsample_yswavfile.h"
//#include "ico_apf_log.h"
#include "ico_log.h"

YsWavFile::YsWavFile()
{
    dat = NULL;
    Initialize();
}

YsWavFile::~YsWavFile()
{
    if (dat != NULL) {
        delete[] dat;
    }
}

void YsWavFile::Initialize(void)
{
    if (dat != NULL) {
        delete[] dat;
        dat = NULL;
    }

    stereo = YSFALSE;
    bit = 16;
    rate = 44100;
    sizeInBytes = 0;
    isSigned = YSTRUE;
}

unsigned int YsWavFile::NTimeStep(void) const
{
    return SizeInByte() / BytePerTimeStep();
}

YSBOOL YsWavFile::Stereo(void) const
{
    return stereo;
}

unsigned int YsWavFile::BytePerTimeStep(void) const
{
    unsigned int nChannel = (YSTRUE == stereo ? 2 : 1);
    return nChannel * BytePerSample();
}

unsigned int YsWavFile::BitPerSample(void) const
{
    return bit;
}

unsigned int YsWavFile::BytePerSample(void) const
{
    return bit / 8;
}

unsigned int YsWavFile::PlayBackRate(void) const
{
    return rate;
}

unsigned int YsWavFile::SizeInByte(void) const
{
    return sizeInBytes;
}

YSBOOL YsWavFile::IsSigned(void) const
{
    return isSigned;
}

const unsigned char *YsWavFile::DataPointer(void) const
{
    return dat;
}

const unsigned char *YsWavFile::DataPointerAtTimeStep(unsigned int ts) const
{
    return dat + ts * BytePerTimeStep();
}

static unsigned GetUnsigned(const unsigned char buf[])
{
    return buf[0] + buf[1] * 0x100 + buf[2] * 0x10000 + buf[3] * 0x1000000;
}

static unsigned GetUnsignedShort(const unsigned char buf[])
{
    return buf[0] + buf[1] * 0x100;
}



YSRESULT YsWavFile::LoadWav(const char fn[])
{
    FILE *fp;

    ICO_DBG("Loading %s", fn);

    fp = fopen(fn, "rb");
    if (fp != NULL) {
        unsigned char buf[256];
        unsigned int l;
        unsigned int fSize, hdrSize, dataSize;

        // Wave Header
        unsigned short wFormatTag, nChannels;
        unsigned nSamplesPerSec, nAvgBytesPerSec;
        unsigned short nBlockAlign, wBitsPerSample, cbSize;


        if (fread(buf, 1, 4, fp) != 4) {
            ICO_DBG("Error in reading RIFF.");
            goto ERREND;
        }
        if (strncmp((char *) buf, "RIFF", 4) != 0) {
            ICO_DBG("Warning: RIFF not found.");
        }


        if (fread(buf, 1, 4, fp) != 4) {
            ICO_DBG("Error in reading file size.");
            goto ERREND;
        }
        fSize = GetUnsigned(buf);
        ICO_DBG("File Size=%d", fSize + 8);

        if (fread(buf, 1, 8, fp) != 8) {
            ICO_DBG("Error in reading WAVEfmt.");
            goto ERREND;
        }
        if (strncmp((char *) buf, "WAVEfmt", 7) != 0) {
            ICO_DBG("Warning: WAVEfmt not found");
        }


        if (fread(buf, 1, 4, fp) != 4) {
            ICO_DBG("Error in reading header size.");
            goto ERREND;
        }
        hdrSize = GetUnsigned(buf);
        ICO_DBG("Header Size=%d", hdrSize);


        //    WORD  wFormatTag;
        //    WORD  nChannels;
        //    DWORD nSamplesPerSec;
        //    DWORD nAvgBytesPerSec;
        //    WORD  nBlockAlign;
        //    WORD  wBitsPerSample;
        //    WORD  cbSize;
        if (fread(buf, 1, hdrSize, fp) != hdrSize) {
            ICO_DBG("Error in reading header.");
            goto ERREND;
        }
        wFormatTag = GetUnsignedShort(buf);
        nChannels = GetUnsignedShort(buf + 2);
        nSamplesPerSec = GetUnsigned(buf + 4);
        nAvgBytesPerSec = GetUnsigned(buf + 8);
        nBlockAlign = GetUnsignedShort(buf + 12);
        wBitsPerSample = (hdrSize >= 16 ? GetUnsignedShort(buf + 14) : 0);
        cbSize = (hdrSize >= 18 ? GetUnsignedShort(buf + 16) : 0);

        ICO_DBG("wFormatTag=%d", wFormatTag);
        ICO_DBG("nChannels=%d", nChannels);
        ICO_DBG("nSamplesPerSec=%d", nSamplesPerSec);
        ICO_DBG("nAvgBytesPerSec=%d", nAvgBytesPerSec);
        ICO_DBG("nBlockAlign=%d", nBlockAlign);
        ICO_DBG("wBitsPerSample=%d", wBitsPerSample);
        ICO_DBG("cbSize=%d", cbSize);



        while (1) {
            if (fread(buf, 1, 4, fp) != 4) {
                ICO_DBG("Error while waiting for data.");
                goto ERREND;
            }

            if ((buf[0] == 'd' || buf[0] == 'D')
                && (buf[1] == 'a' || buf[1] == 'A') && (buf[2] == 't'
                                                        || buf[2] == 'T')
                && (buf[3] == 'a' || buf[3] == 'A')) {
                break;
            }
            else {
                ICO_DBG("Skipping %c%c%c%c", buf[0], buf[1], buf[2],
                            buf[3]);
                if (fread(buf, 1, 4, fp) != 4) {
                    ICO_DBG("Error while skipping unknown block.");
                    goto ERREND;
                }



                l = GetUnsigned(buf);
                if (fread(buf, 1, l, fp) != l) {
                    ICO_DBG("Error while skipping unknown block.");
                    goto ERREND;
                }
            }
        }


        if (fread(buf, 1, 4, fp) != 4) {
            ICO_DBG("Error in reading data size.");
            goto ERREND;
        }
        dataSize = GetUnsigned(buf);
        ICO_DBG("Data Size=%d (0x%x)", dataSize, dataSize);

        dat = new unsigned char[dataSize];
        if ((l = fread(dat, 1, dataSize, fp)) != dataSize) {
            ICO_DBG("Warning: File ended before reading all data.");
            ICO_DBG("  %d (0x%x) bytes have been read", l, l);
        }

        stereo = (nChannels == 2 ? YSTRUE : YSFALSE);
        bit = wBitsPerSample;
        sizeInBytes = dataSize;
        rate = nSamplesPerSec;

        if (wBitsPerSample == 8) {
            isSigned = YSFALSE;
        }
        else {
            isSigned = YSTRUE;
        }

        fclose(fp);
        return YSOK;
    }
    return YSERR;

  ERREND:
    ICO_DBG("Err!");
    if (fp != NULL) {
        fclose(fp);
    }
    return YSERR;
}


YSRESULT YsWavFile::ConvertTo16Bit(void)
{
    if (bit == 16) {
        return YSOK;
    }
    else if (bit == 8) {
        if (sizeInBytes > 0 && dat != NULL) {
            unsigned char *newDat = new unsigned char[sizeInBytes * 2];
            for (unsigned int i = 0; i < sizeInBytes; i++) {
                newDat[i * 2] = dat[i];
                newDat[i * 2 + 1] = dat[i];
            }
            delete[] dat;
            dat = newDat;

            sizeInBytes *= 2;
            bit = 16;
        }
        return YSOK;
    }
    return YSERR;
}

YSRESULT YsWavFile::ConvertTo8Bit(void)
{
    if (bit == 8) {
        return YSOK;
    }
    else if (bit == 16) {
        unsigned char *newDat = new unsigned char[sizeInBytes / 2];
        for (unsigned int i = 0; i < sizeInBytes; i += 2) {
            newDat[i / 2] = dat[i];
        }
        if (dat != NULL) {
            delete[] dat;
        }
        dat = newDat;
        bit = 8;
        sizeInBytes /= 2;
        return YSOK;
    }
    return YSERR;
}

YSRESULT YsWavFile::ConvertToStereo(void)
{
    if (stereo == YSTRUE) {
        return YSOK;
    }
    else {
        if (bit == 8) {
            unsigned char *newDat = new unsigned char[sizeInBytes * 2];
            for (unsigned int i = 0; i < sizeInBytes; i++) {
                newDat[i * 2] = dat[i];
                newDat[i * 2 + 1] = dat[i];
            }
            if (dat != NULL) {
                delete[] dat;
            }
            dat = newDat;
            stereo = YSTRUE;
            sizeInBytes *= 2;
            return YSOK;
        }
        else if (bit == 16) {
            unsigned char *newDat = new unsigned char[sizeInBytes * 2];
            for (unsigned int i = 0; i < sizeInBytes; i += 2) {
                newDat[i * 2] = dat[i];
                newDat[i * 2 + 1] = dat[i + 1];
                newDat[i * 2 + 2] = dat[i];
                newDat[i * 2 + 3] = dat[i + 1];
            }
            if (dat != NULL) {
                delete[] dat;
            }
            dat = newDat;
            stereo = YSTRUE;
            sizeInBytes *= 2;
            return YSOK;
        }
    }
    return YSERR;
}

YSRESULT YsWavFile::Resample(unsigned int newRate)
{
    if (rate != newRate) {
        const size_t nChannel = (YSTRUE == stereo ? 2 : 1);
        const size_t bytePerSample = bit / 8;
        const size_t bytePerTimeStep = nChannel * bytePerSample;
        const size_t curNTimeStep = sizeInBytes / bytePerTimeStep;

        const size_t newNTimeStep = curNTimeStep * newRate / rate;
        const size_t newSize = newNTimeStep * bytePerTimeStep;

        unsigned char *newDat;
        newDat = (0 < newSize ? new unsigned char[newSize] : NULL);

        if (NULL != newDat) {
            for (size_t ts = 0; ts < newNTimeStep; ts++) {
                double oldTimeStepD = (double) curNTimeStep * (double) ts /
                    (double) newNTimeStep;
                size_t oldTimeStep = (size_t) oldTimeStepD;
                double param = fmod(oldTimeStepD, 1.0);
                unsigned char *newTimeStepPtr = newDat + ts * bytePerTimeStep;

                for (size_t ch = 0; ch < nChannel; ++ch) {
                    if (curNTimeStep - 1 <= oldTimeStep) {
                        const int value =
                            GetSignedValue(curNTimeStep - 1, ch);
                        SetSignedValue(newTimeStepPtr + bytePerSample * ch,
                                       value);
                    }
                    else if ((0 == oldTimeStep) ||
                             (curNTimeStep - 2 <= oldTimeStep)) {
                        const double value[2] = {
                            (double) GetSignedValue(oldTimeStep, ch),
                            (double) GetSignedValue(oldTimeStep + 1, ch)
                        };
                        const int newValue = (int)
                            (value[0] * (1.0 - param) + value[1] * param);
                        SetSignedValue(newTimeStepPtr + bytePerSample * ch,
                                       newValue);
                    }
                    else {
                        const double v[4] = {
                            // At x=-1.0
                            (double) GetSignedValue(oldTimeStep - 1, ch),
                            // At x= 0.0
                            (double) GetSignedValue(oldTimeStep, ch),
                            // At x= 1.0
                            (double) GetSignedValue(oldTimeStep + 1, ch),
                            // At x= 2.0
                            (double) GetSignedValue(oldTimeStep + 2, ch)
                        };

                        // Cubic interpolation.  Linear didn't work well.
                        // axxx+bxx+cx+d=e
                        // x=-1  -> -a+b-c+d=v0   (A)
                        // x= 0  ->        d=v1   (B)
                        // x= 1  ->  a+b+c+d=v2   (C)
                        // x= 2  -> 8a+4b+2c+d=v3 (D)
                        //
                        // (B) =>  d=v1;
                        // (A)+(C) => 2b+2d=v0+v2  => b=(v0+v2-2d)/2
                        //
                        // (D)-2*(B) =>  6a+2b-d=v3-2*v2
                        //           =>  a=(v3-2*v2-2b+d)/6

                        const double d = v[1];
                        const double b = (v[0] + v[2] - 2.0 * d) / 2.0;
                        const double a =
                            (v[3] - 2.0 * v[2] - 2.0 * b + d) / 6.0;
                        const double c = v[2] - a - b - d;

                        double newValue = a * param * param * param
                            + b * param * param + c * param + d;
                        SetSignedValue(newTimeStepPtr + bytePerSample * ch,
                                       (int) newValue);
                    }
                }
            }
        }

        rate = newRate;
        if (dat != NULL) {
            delete[] dat;
        }
        dat = newDat;
        sizeInBytes = newSize;
    }
    return YSOK;
}

YSRESULT YsWavFile::ConvertToMono(void)
{
    if (YSTRUE == stereo) {
        const size_t bytePerSample = bit / 8;
        const size_t bytePerTimeStep = 2 * bytePerSample;
        const size_t nTimeStep = sizeInBytes / bytePerTimeStep;

        const size_t newSize = nTimeStep * bytePerSample;

        unsigned char *newDat;
        newDat = (0 < newSize ? new unsigned char[newSize] : NULL);
        if (NULL != newDat) {
            for (size_t ts = 0; ts < nTimeStep; ts++) {
                const int newValue =
                    (GetSignedValue(ts, 0) + GetSignedValue(ts, 1)) / 2;
                unsigned char *const newTimeStepPtr =
                    newDat + ts * bytePerSample;
                SetSignedValue(newTimeStepPtr, newValue);
            }

            if (dat != NULL) {
                delete[] dat;
            }
            dat = newDat;
            sizeInBytes = newSize;
            stereo = YSFALSE;

            return YSOK;
        }
    }
    return YSERR;
}

YSRESULT YsWavFile::ConvertToSigned(void)
{
    if (isSigned == YSTRUE) {
        return YSOK;
    }
    else {
        if (bit == 8) {
            for (unsigned int i = 0; i < sizeInBytes; i++) {
                dat[i] -= 128;
            }
        }
        else if (bit == 16) {
            for (unsigned int i = 0; i < sizeInBytes - 1; i += 2) {
                int d;
                d = dat[i] + dat[i + 1] * 256;
                d -= 32768;
                dat[i] = d & 255;
                dat[i + 1] = (d >> 8) & 255;
            }
        }
        isSigned = YSTRUE;
    }
    return YSOK;
}

YSRESULT YsWavFile::ConvertToUnsigned(void)
{
    if (isSigned != YSTRUE) {
        return YSOK;
    }
    else {
        if (bit == 8) {
            for (unsigned int i = 0; i < sizeInBytes; i++) {
                dat[i] += 128;
            }
        }
        else if (bit == 16) {
            for (unsigned int i = 0; i < sizeInBytes - 1; i += 2) {
                int d = dat[i] + dat[i + 1] * 256;
                if (d >= 32768) {
                    d -= 65536;
                }
                d += 32768;
                dat[i] = d & 255;
                dat[i + 1] = (d >> 8) & 255;
            }
        }
        isSigned = YSFALSE;
    }
    return YSOK;
}

int YsWavFile::GetNumChannel(void) const
{
    return (YSTRUE == stereo ? 2 : 1);
}

int YsWavFile::GetNumSample(void) const
{
    return sizeInBytes * 8 / bit;
}

int YsWavFile::GetNumSamplePerChannel(void) const
{
    return GetNumSample() / GetNumChannel();
}

size_t YsWavFile::GetUnitSize(void) const
{
    return BytePerSample() * GetNumChannel();
}

size_t YsWavFile::GetSamplePosition(int atIndex) const
{
    return atIndex * GetNumChannel() * (bit / 8);
}

int YsWavFile::GetSignedValue(int atTimeStep, int channel) const
{
    const size_t sampleIdx = GetSamplePosition(atTimeStep);
    const size_t unitSize = GetUnitSize();

    if ((sampleIdx + unitSize <= sizeInBytes) &&
        (0 <= channel) && (channel < GetNumChannel())) {
        int rawSignedValue = 0;
        size_t offset = sampleIdx + channel * BytePerSample();
        switch (BitPerSample()) {
        case 8:
            if (YSTRUE == isSigned) {
                rawSignedValue = dat[offset];
                if (128 <= rawSignedValue) {
                    rawSignedValue -= 256;
                }
            }
            else {
                rawSignedValue = dat[offset] - 128;
            }
            break;
        case 16:
            // Assume little endian
            rawSignedValue = dat[offset] + 256 * dat[offset + 1];
            if (YSTRUE == isSigned) {
                if (32768 <= rawSignedValue) {
                    rawSignedValue -= 65536;
                }
            }
            else {
                rawSignedValue -= 32768;
            }
            break;
        }
        return rawSignedValue;
    }
    return 0;
}

void YsWavFile::SetSignedValue(unsigned char *savePtr, int rawSignedValue)
{
    switch (bit) {
    case 8:
        if (rawSignedValue < -128) {
            rawSignedValue = -128;
        }
        else if (127 < rawSignedValue) {
            rawSignedValue = 127;
        }
        if (YSTRUE == isSigned) {
            if (0 > rawSignedValue) {
                rawSignedValue += 256;
            }
            *savePtr = (unsigned char) rawSignedValue;
        }
        else {
            rawSignedValue += 128;
            *savePtr = (unsigned char) rawSignedValue;
        }
        break;
    case 16:
        if (-32768 > rawSignedValue) {
            rawSignedValue = -32768;
        }
        else if (32767 < rawSignedValue) {
            rawSignedValue = 32767;
        }

        if (YSTRUE == isSigned) {
            if (0 > rawSignedValue) {
                rawSignedValue += 65536;
            }
        }
        else {
            rawSignedValue += 32768;
        }

        // Assume little endian (.WAV is supposed to use little endian).
        savePtr[0] = (rawSignedValue & 255);
        savePtr[1] = ((rawSignedValue >> 8) & 255);
        break;
    }
}
