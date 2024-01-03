// hashtable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <string>
#include "hashtable.h"
#include <unordered_map>


class strtable :public hashtable<std::string, hashtable<std::string, const char*>> {

};



int main()
{
	const int size = sizeof(hashtable_node< const char*, const char*>);
	std::unordered_map<const char*, const char*> t1;
	hashtable<const char*, hashtable<const char*,const char*>> ht;
	auto node = ht["左"].add("下","1");
	ht["1"]["2"] = "2";
	auto& node2 = ht["1"]["2"];
	ht["1"]["2"] = "3";
	const char*str = ht["1"]["2"];
	printf("左下=%s\n", ht["左"]["下"]);
	ht["左"]["下"] = "黄河之水天上来";
	ht["左"]["下"] = "奔流到海不复回";
	ht["上"]["右"] = "下";
	
	printf("左下=%s,右上=%s\n",ht["左"]["下"], ht["上"]["右"]);

	hashtable<const char*, void> ht2;
	auto& vnode = ht2["1"];

    std::cout << "Hello World!\n";
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
