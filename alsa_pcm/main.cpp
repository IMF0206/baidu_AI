/*************************************************************************
	> File Name: lrecord.c
	> Author: 
	> Mail: 
	> Created Time: 2018年01月03日 星期三 14时59分14秒
 ************************************************************************/

//File   : lrecord.c  
//Author : Loon <sepnic@gmail.com>  
  
#include <stdio.h>  
#include <malloc.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <string.h>  
#include <getopt.h>  
#include <fcntl.h>  
#include <ctype.h>  
#include <errno.h>  
#include <limits.h>  
#include <time.h>  
#include <locale.h>  
#include <sys/unistd.h>  
#include <sys/stat.h>  
#include <sys/types.h>  
#include <alsa/asoundlib.h>  
#include <assert.h>  
#include "wav_parser.h"  
#include "sndwav_common.h"

#include "speech.h"
#include "nlp.h"
#include <iostream>

std::string app_id = "10613893";
std::string api_key = "XWzWZV7Q8gWpNyIjyMZ5qbAG";
std::string secret_key = "uVu1S5KKfp5lxxmzjY9psXDBR2iOG2Mm";

std::string app_nlpid = "10615361";
std::string api_nlpkey = "w4nfGmqG8lgOzgLCwvj4Fq8T";
std::string secret_nlpkey = "VY0A4DVKLKLPtVP14Cc96HZehB26PelU";

aip::Speech client(app_id, api_key, secret_key);
aip::Nlp nlpclient(app_nlpid, api_nlpkey, secret_nlpkey);
  
#define DEFAULT_CHANNELS         (1)
#define DEFAULT_SAMPLE_RATE      (16000)
#define DEFAULT_SAMPLE_LENGTH    (16)  
#define DEFAULT_DURATION_TIME    (6)
  
int SNDWAV_PrepareWAVParams(WAVContainer_t *wav)  
{  
    assert(wav);  
  
    uint16_t channels = DEFAULT_CHANNELS;  
    uint16_t sample_rate = DEFAULT_SAMPLE_RATE;  
    uint16_t sample_length = DEFAULT_SAMPLE_LENGTH;  
    uint32_t duration_time = DEFAULT_DURATION_TIME;  
  
    /* Const */  
    wav->header.magic = WAV_RIFF;  
    wav->header.type = WAV_WAVE;  
    wav->format.magic = WAV_FMT;  
    wav->format.fmt_size = LE_INT(16);  
    wav->format.format = LE_SHORT(WAV_FMT_PCM);  
    wav->chunk.type = WAV_DATA;  
  
    /* User definition */  
    wav->format.channels = LE_SHORT(channels);  
    wav->format.sample_rate = LE_INT(sample_rate);  
    wav->format.sample_length = LE_SHORT(sample_length);  
  
    /* See format of wav file */  
    wav->format.blocks_align = LE_SHORT(channels * sample_length / 8);  
    wav->format.bytes_p_second = LE_INT((uint16_t)(wav->format.blocks_align) * sample_rate);  
      
    wav->chunk.length = LE_INT(duration_time * (uint32_t)(wav->format.bytes_p_second));  
    wav->header.length = LE_INT((uint32_t)(wav->chunk.length) +
        sizeof(wav->chunk) + sizeof(wav->format) + sizeof(wav->header) - 8);  
  
    return 0;  
}  
  
void SNDWAV_Record(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav, int fd)  
{  
    off64_t rest;  
    size_t c, frame_size;  
      
    if (WAV_WriteHeader(fd, wav) < 0) {  
        exit(-1);  
    }  
  
    rest = wav->chunk.length;  
    while (rest > 0) {  
        c = (rest <= (off64_t)sndpcm->chunk_bytes) ? (size_t)rest : sndpcm->chunk_bytes;  
        frame_size = c * 8 / sndpcm->bits_per_frame;  
        if (SNDWAV_ReadPcm(sndpcm, frame_size) != frame_size)  
            break;  
          
        if (write(fd, sndpcm->data_buf, c) != c) {  
            fprintf(stderr, "Error SNDWAV_Record[write]/n");  
            exit(-1);  
        }
  
        rest -= c;  
    }  
}

void asr(aip::Speech client, const char* filepath)
{
    // 无可选参数调用接口
    std::string file_content;
    aip::get_file_content(filepath, &file_content);
//    Json::Value result = client.recognize(file_content, "wav", 16000, aip::null);
//    std::cout << result << std::endl;

    // 如果需要覆盖或者加入参数
    std::map<std::string, std::string> options;
    options["lan"] = "en";
    Json::Value result = client.recognize(file_content, "wav", 16000, options);
    std::cout << result << std::endl;
}

void f_error(SNDPCMContainer_t record, int fd, const char* filename) {
    close(fd);
    remove(filename);
    if (record.data_buf) free(record.data_buf);
    if (record.log) snd_output_close(record.log);
    if (record.handle) snd_pcm_close(record.handle);
    return;
}
  
int main(int argc, char *argv[])
{
    char *filename;
    char *devicename = "default";
    int fd;
    WAVContainer_t wav;
    SNDPCMContainer_t record;

    if (argc != 2) {
        fprintf(stderr, "Usage: ./lrecord <FILENAME>/n");
        return -1;
    }

    memset(&record, 0x0, sizeof(record));

    filename = argv[1];
    remove(filename);
    if ((fd = open(filename, O_WRONLY | O_CREAT, 0644)) == -1) {  
        fprintf(stderr, "Error open: [%s]/n", filename);  
        return -1;  
    }  
  
    if (snd_output_stdio_attach(&record.log, stderr, 0) < 0) {  
        fprintf(stderr, "Error snd_output_stdio_attach/n");  
        f_error(record, fd, filename);
    }  
  
    if (snd_pcm_open(&record.handle, devicename, SND_PCM_STREAM_CAPTURE, 0) < 0) {  
        fprintf(stderr, "Error snd_pcm_open [ %s]/n", devicename);  
        f_error(record, fd, filename);
    }  
  
    if (SNDWAV_PrepareWAVParams(&wav) < 0) {  
        fprintf(stderr, "Error SNDWAV_PrepareWAVParams/n");  
        f_error(record, fd, filename);
    }  
  
    if (SNDWAV_SetParams(&record, &wav) < 0) {  
        fprintf(stderr, "Error set_snd_pcm_params/n");  
        f_error(record, fd, filename);
    }  
    snd_pcm_dump(record.handle, record.log);  
  
    SNDWAV_Record(&record, &wav, fd);  
  
    snd_pcm_drain(record.handle);
  
    close(fd);  
    free(record.data_buf);  
    snd_output_close(record.log);  
    snd_pcm_close(record.handle);
    std::map<std::string, std::string> options;
    options["lan"] = "en";
    asr(client, filename);
    return 0;
  
//Err:
//    close(fd);
//    remove(filename);
//    if (record.data_buf) free(record.data_buf);
//    if (record.log) snd_output_close(record.log);
//    if (record.handle) snd_pcm_close(record.handle);
//    return -1;
}  
