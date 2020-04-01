//
// Created by 王庆 on 2020-04-01.
//

#ifndef OPENSLADUIO_RECORDBUFFER_H
#define OPENSLADUIO_RECORDBUFFER_H

/**
 * 流式录音 就相当于在录制过程中，需要连续的去录制，
 *
 * 那么怎么去做， 可以通两个buffer缓冲区去交替的录制数据然后写入到文件中
 *
 * 分为当前写入文件的buffer 和 要去录制数据的buffer
 *
 * 当写入文件的buffer填充了数据 要将数据写入到文件中，写入文件之后 释放内存
 *
 * 此时录制数据的buffer也填充了数据 即下一步将数据写入到文件中，
 *
 * 那么 两个角色交替进行，完成写入文件的buffer 继续填充数据，录制数据的buffer 将数据写入到文件中，
 *
 * 形成一种连续的录制过程。
 *
 * 这个就要考虑到维护索引，二维数组。 二维数组默认维护了索引，来标识当前处于哪个状态的buffer
 *
 *
 * C++ 动态二维数组的创建 https://blog.csdn.net/bqw18744018044/article/details/81665898?depth_1-utm_source=distribute.pc_relevant_right.none-task&utm_source=distribute.pc_relevant_right.none-task
 *
 *  二维数组  a[2][3]={{1,2,3},{4,5,6}};指针p有如下几种表达形式：
 *
 * ：int (*p)[3]=a  (或&a[0]）; 意思是定义一个指向3个int类型变量的指针。p代表二维数组中第一个一维数组a[0]的首地址，指针加1代表移动到下一个一维数组a[1]的首地址。
 *
 *
 * 1。使用一维数组模型二维数组
 *
 *      int a0 = {1,2,3,4};
 *      for(int i=0;i<rows;i++){
 *          for(int j=0;j<colums;j++){
 *              cout<<a0[i*columns+j]<<" ";//a0[i*columns+j]等价于a0[i][j]
 *          }
 *
 *      }
 *
 * 2. 静态二位数组
 *
 *     int a[2][2] ={{3,4},{1,2}};
 *
 *
 * 3.动态二维数组
 *  //申请空间 [] 优先级大于* 所以定义的是一个数组
 *  int** a = new int*[rows];
 *  for(int i=0;i<rows;i++){
 *      a[i]=new int[colums];
 *
 *   }
 *
 *
 */
class RecordBuffer {


public:
    //声明一个二维数组 以动态分配的方式来创建
    int index = -1;
    short **buffer;

public:
    RecordBuffer(int bufferSize);

    ~RecordBuffer();

    /**
     * 得到当前录制的buffer
     */
     short* getCurrentRecordBuffer();

     /**
      * 得到一个新的录制buffer
      */
      short* getNewRecordBuffer();

};


#endif //OPENSLADUIO_RECORDBUFFER_H
