#!/usr/bin/env python
#-*- encoding:utf-8 -*-
import os
import sys

'''
此处定义的宏内容, 应该和~/smartDNS/src/include/util_glb.h中为添加
统计模块添加的宏名字保持一致
'''
FILE_FILTER = '.c'
STAT_MACRO_FILTER = 'CREATE_STATISTICS'
LOG_MACRO_FILTER =  'SDNS_STAT_'
FUNC_BEG_MACRO_FILTER = 'STAT_FUNC_BEGIN'
FUNC_END_MACRO_FILTER = 'STAT_FUNC_END'
STAT_GLB_H_FILE = '/src/include/statistics_glb.h'
STAT_C_FILE = '/src/log/statistics.c'


def create_stat_c_file(f_list, f_name):
    '''
    生成统计模块儿需要的.c文件
    @param f_list: sort_func_line_dict()生成的排序链表
    @param f_name: 待创建的文件
    @retval: void
    '''
    '''
    文件的格式如下:

    #include "statistics_glb.h

    /*函数调用关系, 方便更美观的debug输出 */
    CALL_RELATION g_call_retions[] = {
        {"func1", "call-func1, call_func2, ..."},
        {NULL, NULL}
    };

    /* 注释信息, 方便输出阅读, 以"file label_linex"为索引 */
    STAT_INFO g_stat_infos[] = {
        {"xxx.c", "mod_x", "func1", "linex"},
        {NULL, NULL, NULL, NULL}
    };
    '''
    #f_val["file"]格式[file label, mod name, file name]
    FILE_LABEL_INDEX = 0
    MOD_NAME_INDEX = 1
    FILE_NAME_INDEX = 2

    with open(f_name, 'w') as fp:
        #开头
        fp.write('#include "util_glb.h"\n')
        fp.write('#include "statistics_glb.h"\n')
        fp.write('\n')

        #生成LOG_CALL_RELATION, 用于debug调试
        fp.write('\n')
        fp.write('/* 函数调用关系, 方便更美观的debug输出 */\n')
        fp.write('CALL_RELATION g_call_relations[] = {\n')
        if f_list:
            for func, f_val in f_list:
                fp.write('    {')
                fp.write('"' + func + '", ')
                fp.write('"{0}"'.format(','.join(f_val['call_fun'])))
                fp.write('},')
                fp.write('\n')
        fp.write('    {NULL, NULL}\n')
        fp.write('};\n')

        #生成STAT_INFO, 用于debug调试
        fp.write('\n')
        fp.write('/* 注释信息, 方便输出阅读, 以enum_func_line为索引 */\n')
        fp.write('STAT_INFO g_stat_infos[] = {\n')
        if f_list:
            for func, f_val in f_list:
                for line in f_val['stat_line']:
                        fp.write('    {')
                        fp.write('"' + f_val['file'][FILE_NAME_INDEX] + '", ')
                        fp.write('"' + f_val['file'][MOD_NAME_INDEX] + '", ')
                        fp.write('"' + func + '", ')
                        fp.write('"{0}"'.format(line))
                        fp.write('},')
                        fp.write('\n')
        fp.write('    {NULL, NULL, NULL, NULL}\n')
        fp.write('};\n')

        #结尾
        fp.write('\n')
        fp.flush()

def create_stat_h_file(f_list, f_name):
    '''
    生成统计模块儿需要的.h文件
    @param f_list: sort_func_line_dict()生成的排序链表
    @param f_name: 待创建的文件
    @retval: void
    '''
    '''
    文件的格式如下:
    #ifndef STATISTICS_GLB_H
    #define STATISTICS_GLB_H

    /* 模块儿索引, 模块儿名来自宏CREATE_STATISTICS() */
    enum {
        mod_x = 0,
        mod_y,
        mod_max
    };

    /* SDNS_LOG_XXX所在行全局排序索引 */
    enum {
        file label + linex = 0,
        file label + liney,
        ...
        enum_file_label_line_max
    };

    /*函数调用关系, 方便更美观的debug输出 */
    typedef struct st_call_relation {
        char *func;           /* 函数名 */
        char *call_func;      /* 调用函数 */
    } CALL_RELATION;
    extern CALL_RELATION g_call_retions[];

    /* 注释信息, 方便输出阅读, 以"file label_linex"为索引 */
    typedef struct st_log_info {
        char *f_name;           /* 文件名 */
        char *mod_name;         /* 模块儿名 */
        char *func;             /* 函数名 */
        char *line;             /* 所在行 */
        char *print_pad;        /* 打印填充 */
    } STAT_INFO;
    extern STAT_INFO g_stat_infos[];

    #endif
    '''

    #f_val["file"]格式[file label, mod name, file name]
    FILE_LABEL_INDEX = 0
    MOD_NAME_INDEX = 1
    FILE_NAME_INDEX = 2

    mod_name_list = []                  #盛放所有模块儿名

    with open(f_name, 'w') as fp:
        #开头
        fp.write('#ifndef STATISTICS_GLB_H\n')
        fp.write('#define STATISTICS_GLB_H\n')
        fp.write('\n')

        #生成enum, 用于各模块儿
        fp.write('\n')
        fp.write('/* 模块儿索引, 模块儿名来自宏CREATE_STATISTICS() */\n')
        if f_list:
            #f_val["file"]格式[file label, mod name, file name]
            for func, f_val in f_list:
                mod_name = f_val["file"][MOD_NAME_INDEX]
                if mod_name in mod_name_list:
                    continue
                mod_name_list.append(mod_name)
        fp.write('enum {\n')
        if mod_name_list:
            for mod_name in mod_name_list:
                fp.write('    {0},\n'.format(mod_name))
        fp.write('    enum_mod_max\n')
        fp.write('};\n')
        fp.write('\n')

        #生成enum, 用于描述待跟踪统计的行
        fp.write('\n')
        fp.write('/* SDNS_LOG_XXX所在行索引, 文件label+行号,来自各文件 */\n')
        line_id = 0;
        fp.write('enum {\n')
        if f_list:
            for func, f_val in f_list:
                #f_val['stat_line']格式: [linex, liney, ..]
                f_label = f_val['file'][FILE_LABEL_INDEX]
                for line in f_val['stat_line']:
                    fp.write('    {0}{1},\n'.format(f_label, line))
        fp.write('    enum_file_label_line_max\n')
        fp.write('};\n')

        #定义函数调用关系结构
        fp.write('\n')
        fp.write('/* 函数调用关系, 方便更美观的debug输出 */\n')
        fp.write('typedef struct st_call_relation {\n')
        fp.write('    char *func;           /* 函数名 */\n')
        fp.write('    char *call_func;      /* 调用函数 */\n')
        fp.write('} CALL_RELATION;\n')
        fp.write('extern CALL_RELATION g_call_retions[];\n')

        #生成STAT_INFO, 用于debug调试
        fp.write('\n')
        fp.write('/* 注释信息, 方便输出阅读, 以enum_func_line为索引 */\n')
        fp.write('typedef struct st_log_info {\n')
        fp.write('    char *f_name;         /* 文件名 */\n')
        fp.write('    char *mod_name;       /* 模块名 */\n')
        fp.write('    char *func;           /* 函数名 */\n')
        fp.write('    char *line;           /* 行号 */\n')
        fp.write('} STAT_INFO;\n')
        fp.write('extern STAT_INFO g_stat_infos[];\n')

        #结尾
        fp.write('\n')
        fp.write('#endif\n')
        fp.flush()

def print_sorted_func_line_list(sorted_list):
    '''
    打印sort_func_line_dict()函数返回的排序数组
    @param sorted_list: 按照调用次数的排序数组
    @retval: void
    '''
    for func,f_val in sorted_list:
        print(func)
        for label,l_val in f_val.items():
            print('\t{0}:\t{1}'.format(label, l_val))

def unsort_func_line_dict(f_l_dict):
    '''
    字典信息转换为列表信息为后续生成全局.h文件服务
    @param f_l_dict: 待处理的信息字典
    @retval: 列表
    '''
    final_list = []
    #参数检测
    if not f_l_dict:
        print('!!!Empty dict')
        return []

    for func, f_val in f_l_dict.items():
        final_list.append([func, f_val])

    #返回
    return final_list

def sort_func_line_dict(f_l_dict):
    '''
    按照函数调用链排序, 为后续生成全局.h文件服务
    @param f_l_dict: 待处理的信息字典
    @retval: []/排序后的链表, 按照调用层次依次排序, 以便后续debug层次输出
            [
                [fir函数名, {file:[], call_fun:[], stat_line:[]}],
                    [called_by_'fir'_1, {}],
                        [called_by_'called_by_fir_1'_1, {...}],
                        ...
                        [called_by_'called_by_fir_1'_b, {...}],
                    ...
                    [called_by_'fir'_a, {}],
                        [called_by_'called_by_fir_a'_1, {...}],
                        ...
                        [called_by_'called_by_fir_a'_c, {...}],
            ]
    '''
    final_list = []
    func = None

    #参数检测
    if not f_l_dict:
        print('!!!Empty dict')
        return []

    #排序
    had_sorted = []                     #已经排序的项
    need_sort = False                   #是否需要继续排序
    while len(f_l_dict) != 1:           #排序完毕0, 最终1个起始点
        key = None
        tmp_list = []

        #找到未排序的key
        for key,key_val in f_l_dict.items():
            if key in had_sorted:       #略过已排过序的
                continue
            if not key_val['call_fun']: #最底层, 无调用, 排序
                need_sort = True
                break

            need_sort = True            #非最底层, 尚有底层需要排序, 等待
            for key_val_f in key_val['call_fun']:
                if key_val_f not in had_sorted:
                    need_sort = False
                    break
            if not need_sort:
                continue

            break                       #非最底层, 调用对象业已排序完毕, 排序

        if need_sort:
            need_sort = False
        else:
            break                       #排序完毕1, 最终多个并列起始点

        #加入已排序队列
        had_sorted.append(key)
        key_val = f_l_dict.pop(key)

        #加入自身
        tmp_list.append([key, key_val])

        #print('\n')
        #print(key)
        #print(key_val)
        if key_val['call_fun']:         #加入已排序的调用函数
            for call_f in key_val['call_fun']:
                call_f_val = f_l_dict.pop(call_f)
                #print('\n')
                #print(call_f)
                #print(call_f_val)
                if call_f in had_sorted:
                    tmp_list.extend(call_f_val)
                else:
                    print("WOULD NOT happen, DEBUG it!!!")

        #插入f_l_dict{}
        f_l_dict[key] = tmp_list

    #形成最终的排序链表
    #print('\n\n最终的起始函数数为: {0}'.format(len(f_l_dict)))
    for func, f_val in f_l_dict.items():
        final_list.extend(f_val)

    #返回
    return final_list

def normalize_func_line_dict(f_l_dict):
    '''
    去除无效的调用函数, 如系统调用等
    @param f_l_dict: 待处理的信息字典
    @retval: {}/处理后的信息字典
    '''
    #参数检测
    if not f_l_dict:
        print('!!!Empty dict')
        return {}

    #去除无效调用
    for func,f_val in f_l_dict.items():
        tmp_f_val = []
        for c_f in f_val['call_fun']:
            if c_f in f_l_dict:
                tmp_f_val.append(c_f)
                continue
            print('!!!remove UNexisted call func: {0}'.format(c_f))
        f_l_dict[func]['call_fun'] = tmp_f_val

    return f_l_dict

def print_func_line_dict(f_l_dict):
    '''
    打印get_log_func_and_line()函数返回的字典
    @param f_l_dict: 待打印的字典
    @retval: 无
    '''
    if not f_l_dict:
        print("Empty dict")
        return

    for func, func_val in f_l_dict.items():
        print("{0}".format(func))
        for label, l_list in func_val.items():
            '''
            if label != 'call_fun':
                continue
            '''
            print("\t{0}{1}".format(label, l_list))

def get_func(line):
    '''
    提取当前行中的函数名
    @param line: 待处理的行
    @retval: []/函数名列表

    @NOTE
        1) 此函数可能有bug, 需要日后持续跟进
        2) 注释中尽量不要带有函数调用
    '''
    func = []

    #排除注释
    if line.strip()[0:1] in ['//', '/*', '*/']:
        return func

    #提取
    token_list = line.split('(')
    for token in token_list[:-1]:           #去掉最后一个, 即最后(的右侧字符
        token = token.split(' ').pop().strip()
                                            #每个分隔取最贴近(的字符
                                            #如if (receive_pkt ())
        token = token.split(')').pop().strip()
                                            #如果包含), 则提取此符号后的部分
                                            #如(void)send_pkt()
        token = token.split('[').pop().strip()
                                            #如果包含[, 则提取此符号后的部分
                                            #如&rr->data[get_arr_index()]
        #不能为空
        if token == '':
            continue

        #不能带有特殊字符
        if ')' in token:
            continue
        if '*' in token:
            continue
        if '/' in token:
            continue

        #提取的函数名, 仅包含字符/数字/-/_等(简化为是否为ASCII编码!)
        if len(token) != len(token.decode('utf-8')):
            continue

        #不能为内置字符
        if token in ['if', 'switch', 'for', 'while', '&',
                '<', '>', '||', '&&', '*', '\n', 'sizeof',
                'htons', 'htonl', 'ntohs', 'ntohl', 'printf',
                'assert', 'SDNS_MEMCPY', 'SDNS_MALLOC',
                'SDNS_MEMSET', 'SDNS_REALLOC', 'SDNS_FREE',
                'snprintf', 'inet_ntop', 'fork', 'getpid',
                'SDNS_LOG_ERR', 'SDNS_LOG_DEBUG', 'sendmsg',
                'recvmsg', 'strchr', 'strstr']:
            continue

        #添加到列表
        func.append(token)

    #返回结果
    return func

def get_log_func_and_line(f_list):
    '''
    遍历传入的文件列表, 查找包含SDNS_LOG_XXX宏的行和函数, 并组装成字典
    @param f_list: 待扫描的.c文件列表
    @retval: 字典, 格式如下
            文件名name + 文件名label
            函数名
                统计节点
                调用函数
            dict{
                函数名:{
                    file:[file label, mod name, file name],
                    call_fun:[funx, funy, ..],      /* 可能包含系统调用等 */
                    stat_line:[linex, liney, ..]
                }
            }
    '''
    final_dict = {}

    for f,mod_name,file_label in f_list:
        with open(f, 'r') as fp:
            f = f.split('/').pop()
            func = None
            tmp_dict = {'file':[], 'call_fun':[], 'stat_line':[]}
            for (l_num, line) in enumerate(fp):
                #提取追踪函数头
                if FUNC_BEG_MACRO_FILTER in line:
                    func_l = get_func(line)
                    if func_l and len(func_l)==1:
                        func = func_l[0]
                        tmp_dict['file'] = [file_label, mod_name, f]
                    else:
                        print("追踪函数提取失败: {0}".format(func_l))

                    continue

                #提取追踪函数尾
                if FUNC_END_MACRO_FILTER in line:
                    if func:
                        final_dict[func] = tmp_dict
                    else:
                        print("just {0}".format(FUNC_END_MACRO_FILTER))

                    func = None
                    tmp_dict = {'file':[], 'call_fun':[], 'stat_line':[]}
                    continue

                #提取追踪行
                if LOG_MACRO_FILTER in line:
                    if func:
                        #magic 1: l_num从0开始
                        tmp_dict['stat_line'].append(l_num + 1)
                    continue

                #提取调用函数
                if '(' in line or ')' in line:
                    call_fun = get_func(line)
                    if func and call_fun:
                        tmp_dict['call_fun'].extend(call_fun)
                    continue
    #返回
    return final_dict

def get_file_contain_STAT_MACRO(f_list, engine_mod):
    '''
    遍历传入列表, 查找包含CREATE_STATISTICS()宏的文件, 并返回
    @param f_list: 所有.c文件列表
    @param engine_mod: 引擎模式, 'DEFAULT'/'DPDK'
    @retval: 所有包含指定宏的子列表
                [
                    [file name, mod name, file label],
                    ...
                ]

    @NOTE
        宏格式为 #define CREATE_STATISTICS(mod, file_label)
            第一个参数为[模块儿名], 第二个参数为[文件名label]
    '''
    final_list = []
    remove_f_l = ''

    if engine_mod == 'DEFAULT':
        remove_f_l = 'engine_dpdk_c'
    elif engine_mod == 'DPDK':
        remove_f_l = 'engine_c'
    else:
        print("未知的引擎模式: {0}".format(engine_mod))
        return final_list

    for f in f_list:
        with open(f, 'r') as fp:
            for line in fp:
                if STAT_MACRO_FILTER not in line:
                    continue
                #取宏括号中的参数组合
                res_cp = line.split('(')[1].split(')')[0]
                #取第一个参数
                mod_name = res_cp.split(',')[0].strip()
                #取第二个参数
                file_label = res_cp.split(',')[1].strip()
                if file_label == remove_f_l:
                    continue
                #组合结果
                final_list.append([f, mod_name, file_label])
                break

    return final_list

def walk_dir(cur_dir):
    '''
    recursive walk from cur_dir, and get all [*.c] file, write into [fp]
    @param cur_dir: current directory
    @retval: [.c] file list
    '''
    '''
    walk(top [, topdown=True [, onerror=None] [, followlinks=False]]])

    Generates the filenames in a directory tree by walking the tree
    either top-down or bottom-up. For each directory in the tree
    rooted at directory top (including top itself), yields a three-tuple
    (dirpath, dirnames, filenames)
        dirpath: the path to the directory
        dirnames: name list of subdirectories in dirpath(excluding . + ..)
        filenames: name list of the nondirectory files in dirpath
    Note that the names in the lists do not contain path components
    '''
    c_f_list = []

    for root, dirs, files in os.walk(cur_dir):
        for name in files:
            if name[-2:] != FILE_FILTER:    #magic -2: 取最后两个字符
                continue
            c_f_list.append(os.path.join(root, name))

    return c_f_list

if __name__ == "__main__":
    '''
    遍历[~/smartDNS/src]目录, 查看所有的[.c]文件, 如果文件中包含宏
        CREATE_STATISTICS(mod_xxx)
    则此文件将被统计模块儿跟踪, 因此提取此文件中的信息, 如
        文件名          由目录跟踪函数walk_dir()确定
        文件label       由宏"#define STAT_FILE  dns_c"确定
        跟踪函数        由宏STAT_FUNC_BEGIN/STAT_FUNC_END确定
        跟踪节点        由宏SDNS_LOG_XXX确定
        调用函数        由模式xxx()确定
    并以此信息为基础, 生成文件
        [~/smartDNS/src/include/statistics_glb.h],
        [~/smartDNS/src/log/statistics.c]
    此文件包含了统计模块儿所需要的信息
    '''
    #提取所有的[.c]文件
    #print("当前工作目录: {0}".format(os.getcwd()))
    f_list = walk_dir(os.getcwd() + '/src')

    #过滤带有统计宏的文件---CREATE_STATISTICS(mod_xxx)
    f_list = get_file_contain_STAT_MACRO(f_list, sys.argv[1])
    #print("需要处理的.c文件: {0}".format(f_list))

    #提取统计函数及统计行
    func_line_dict = get_log_func_and_line(f_list)
    #print("\n\n!!!after statistics!!!")
    #print_func_line_dict(func_line_dict)

    #去除无效调用函数, 如系统调用
    func_line_dict = normalize_func_line_dict(func_line_dict)
    #print("\n\n!!!after normalize!!!")
    #print_func_line_dict(func_line_dict)

    #排序, <TAKE CARE!!!>排序输出等功能放置到C代码中实现, 此处只
    #                   提供原始素材
    #sorted_func_line_list = sort_func_line_dict(func_line_dict)
    unsorted_func_line_list = unsort_func_line_dict(func_line_dict)
    #print("\n\n!!!after sorted!!!")
    #print_sorted_func_line_list(sorted_func_line_list)
    #print_sorted_func_line_list(unsorted_func_line_list)

    #生成.c/.h文件
    #create_stat_h_file(sorted_func_line_list, os.getcwd() + STAT_GLB_H_FILE)
    #create_stat_c_file(sorted_func_line_list, os.getcwd() + STAT_C_FILE)
    create_stat_h_file(unsorted_func_line_list, os.getcwd() + STAT_GLB_H_FILE)
    create_stat_c_file(unsorted_func_line_list, os.getcwd() + STAT_C_FILE)



