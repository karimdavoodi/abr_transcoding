#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
/**
 *    moof:
 *      mfhd
            "sequence number": 2
 *      traf
 *          tfhd
                "default sample duration": 100,
 *          tfdt
                "base media decode time": 0
            trun
                "sample count": 60,
                "first sample flags": 64
 *
 *
 *
 *
 *
 *
 */
/*
gst-launch-1.0 videotestsrc pattern=ball num-buffers=2000 
    ! queue ! x264enc key-int-max=60       # every 2 second
    ! 'video/x-h264, framerate=(fraction)30/1' 
    ! mp4mux   fragment-duration=2000      # fragment-duration >= gop, then fragment in gop
    ! filesink location=video.mp4
 */
void write_to_file(unsigned char *buf, int size, const char* file_name);
int main(int argc, char *argv[])
{
    int n;
    int size;
    char frag_name[80];
    char type[5];
    FILE *f;
    unsigned char * buf;
    type[4] = '\0';
    const char *name = "/home/karim/src/VIDOE_LIBS/Bento4/build/tmp/video.mp4";
    f = fopen(name, "rb");
    if(!f){
        printf("Can't open %s\n", argv[1]);
        return -1;
    }
    buf = (unsigned char*) malloc(16000000);
    if(!buf){
        printf("Can't alloc mem\n");
        return -1;
    }
    printf(""
            "#EXTM3U\n"
            "#EXT-X-TARGETDURATION:4\n"
            "#EXT-X-VERSION:3\n");
    int i = 0;
    int frag_seq = 0;
    while(!feof(f)){
        n = fread(&size, 1, 4, f);
        if(n != 4) break;
        memcpy(buf+i, &size, 4);
        size = htonl(size);
        n = fread(type, 1, 4, f);
        if(n != 4) break;
        memcpy(buf+i+4, type, 4);
        fread(buf+i+8, 1, size-8, f);
        i += size;
        if(!strncmp(type, "moov", 4)){
            // Write 'ftyp' and 'moov' in init.mp4
            write_to_file(buf, i, "init.mp4");  
            i = 0;
        }
        if(!strncmp(type, "mdat", 4)){ 
            // Write 'moof' and 'mdat' in f%02d.m4s
            sprintf(frag_name, "f%02d.m4s", frag_seq++);
            write_to_file(buf, i, frag_name);  
            i = 0;
        }
        printf("#EXT-X-ENDLIST\n"); 
    }
}
void write_to_file(unsigned char *buf, int size, const char* file_name)
{
    FILE* f = fopen(file_name, "wb");
    if(!f) return;
    fwrite(buf, size, 1, f);
    fclose(f);
    if(!strcmp(file_name, "init.mp4")){
        printf("#EXT-X-MAP:URI=\"init.mp4\"\n");
    }else
        printf("#EXTINF:2,%s\n%s\n", file_name, file_name);
    
}
