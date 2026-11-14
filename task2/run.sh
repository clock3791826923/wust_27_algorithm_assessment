#!/usr/bin/env bash
command=$1
if [ "$command" = "build" ]; then
    echo "Building the project..."
    mkdir -p build
    cd build
    cmake ..
    make
    #$?:退出状态码 -eq:相等 -nq：不相等 >&2输出错误信息
    if [ $? -eq 0 ]; then
        echo "编译成功，生成可执行文件 app。"
    else
        echo "编译失败。" >&2
        exit 1
    fi
elif [ "$command" = "run" ]; then
    #检查可执行文件app是否存在
    if [ ! -f build/app ]; then
        echo "fail:找到不到可执行文件，请先运行 './run.sh build' 构建项目。" >&2
        exit 1
    fi
    if [ "$2" = "--" ]; then
        shift 2
        ./build/app "$@"
    else
        ./build/app
    fi
elif [ "$command" = "clean" ]; then
    echo "清理构建目录..."
    rm -rf build
    echo "清理完成。"
elif [ "$command" = "help" ]; then
    echo "用法: ./run.sh [command]"
    echo "命令:"
    echo "  build   构建项目"
    echo "  run     运行项目"
    echo "  clean   清理构建目录"
    echo "  help    显示此帮助信息"
else
    echo "无效的命令。请使用 'build' 或 'run'。" >&2
    echo "用法: ./run.sh [command]"
    exit 1
fi