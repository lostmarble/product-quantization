# product quantization
---

This project contains all the tools to find stable points in a picture. Additionally, it can also be used as an picture search
engine.

The project mainly impletes the algorithm introduced by:

 "Product quantization for nearest neighbor search"
 Hervé Jégou, Matthijs Douze and Cordelia Schmid, 2011 TPAMI.

and also use a library(yael) from their project, all rights own to them.


## Commands:

1.提取特征
  使用命令:extracter picture_dir feature_dir
  其中picture_dir是图片存放的文件夹，默认该文件夹只存在图片文件，
feature_dir 是特征存放的文件夹，提取特征前需要清空其中的文件。
  对于图片数据库和查询的图片要分别提取特征

2.训练
  使用命令:pqtrain base_pic_feature_dir
  程序输出为model.dat, database.dat

3.查询
  使用命令:pqsearch  query_pic_feature_dir/query_pic_feature_list_file
  参数可以是查询图片库特征文件夹或者查询图片特征文列表，
  程序加载model.dat和database.dat，进行查询
  第一次查询会在图片库特征文件夹中生成summery.txt文件，如果
图片库特征文件夹有改动，需要先删除这个文件。
  查询的结果存放在当前目录下的search_rslt[thread_id].txt

4.稳定点过滤
  使用命令:stablizer search_result.txt stablize_result.txt
  search_result.txt是查询所得的结果文件，
  stablize_result.txt是稳定点过滤后的结果文件

5.过滤后查询
　使用命令:filter stablize_result.txt out_put_dir
  stablize_result.txt是查询所得的结果文件，
  out_put_dir是新的特征文件存放的目录
  将过滤后的特征重新保存在一个新文件中

## Log:

2012年 12月 04日 星期二 22:03:53 CST
查询与训练分离开，这样训练的模型是通用的，但数据集需要据此建立索引
对于无法一次加载到内存的数据集，可以分开，一部分用来训练模型，另外一部分
可以通过模型与训练集融合，这样数据集的索引就完整了。

2012年 12月 23日 星期日 17:25:46 CST
添加新工具filter,用来对稳定点进行过滤

## Notes:

1. centroid file format
struct {
    int coarse_k;
    float* centroids;/* coarse_k*FEATURE_LEN*/
}
2. all feature file name should end with "_ftr"
3. to run stablizer, make with "install" so that configure file is present in the build dir
4. draw command will draw the stable point with color (255,0,255),unstable points with color (0,255,0)
