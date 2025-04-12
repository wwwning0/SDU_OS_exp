#include "ipc.h"
key_t buff_key;
key_t prod_key;
key_t cons_key;
key_t cget_key;
key_t pput_key;

// 生产消费者共享缓冲区即其有关的变量 key_t buff_key;
int buff_num;
char *buff_ptr;
// 生产者放产品位置的共享指针 key_t pput_key;
int pput_num;
int *pput_ptr;
// 消费者取产品位置的共享指针 key_t cget_key;
int cget_num;
int *cget_ptr;
// 生产者有关的信号量 key_t prod_key;
key_t pmtx_key;
int prod_sem;
int pmtx_sem;
// 消费者有关的信号量 key_t cons_key;
key_t cmtx_key;
int cons_sem;
int cmtx_sem;
int sem_val;
int sem_flg;
int shm_flg;

int main(int argc, char *argv[])
{
    int rate;
    // 可在在命令行第一参数指定一个进程睡眠秒数，以调解进程执行速度
    if (argv[1] != NULL)
        rate = atoi(argv[1]);
    else
        rate = 3;
    // 不指定为 3 秒
    // 共享内存使用的变量
    buff_key = 101;             // 缓冲区任给的键值
    buff_num = 8;               // 缓冲区任给的长度
    pput_key = 102;             // 生产者放产品指针的键值
    pput_num = 1;               // 指针数
    shm_flg = IPC_CREAT | 0644; // 共享内存读写权限
    // 获取缓冲区使用的共享内存，buff_ptr 指向缓冲区首地址
    buff_ptr = (char *)set_shm(buff_key, buff_num, shm_flg);
    // 获取生产者放产品位置指针 pput_ptr
    pput_ptr = (int *)set_shm(pput_key, pput_num, shm_flg);

    // 信号量使用的变量
    prod_key = 201; // 生产者同步信号量键值
    pmtx_key = 202; // 生产者互斥信号量键值
    cons_key = 301; // 消费者同步信号量键值
    cmtx_key = 302; // 消费者互斥信号量键值

    sem_flg = IPC_CREAT | 0644;
    // 生产者同步信号量初值设为缓冲区最大可用量
    sem_val = buff_num;
    // 获取生产者同步信号量，引用标识存 prod_sem
    prod_sem = set_sem(prod_key, sem_val, sem_flg);
    // 消费者初始无产品可取，同步信号量初值设为 0
    sem_val = 0;
    // 获取消费者同步信号量，引用标识存 cons_sem
    cons_sem = set_sem(cons_key, sem_val, sem_flg);
    // 生产者互斥信号量初值为 1
    sem_val = 1;
    // 获取生产者互斥信号量，引用标识存 pmtx_sem
    pmtx_sem = set_sem(pmtx_key, sem_val, sem_flg);
    // 循环执行模拟生产者不断放产品
    while (1)
    {
        // 如果缓冲区满则生产者阻塞
        down(prod_sem);
        // 如果另一生产者正在放产品，本生产者阻塞
        down(pmtx_sem);
        // 用写一字符的形式模拟生产者放产品，报告本进程号和放入的字符及存放的位置
        buff_ptr[*pput_ptr] = 'A' + *pput_ptr;
        sleep(rate);
        printf("%d producer put: %c to Buffer[%d]\n", getpid(), buff_ptr[*pput_ptr], *pput_ptr);
        // 存放位置循环下移
        *pput_ptr = (*pput_ptr + 1) % buff_num;

        // 唤醒阻塞的生产者
        up(pmtx_sem);
        // 唤醒阻塞的消费者
        up(cons_sem);
    }
    return EXIT_SUCCESS;
}