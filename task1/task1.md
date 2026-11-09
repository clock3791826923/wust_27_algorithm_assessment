# 终端美化报告
>把shell从bash换成zsh再美化终端
## 1.设计说明
**第一步：安装zsh**

```bash
sudo apt install zsh
```

**第二步：将bash换成zsh**

```bash
chsh -s $(which zsh)
```

**第三步：安装oh my zsh**

```bash
sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"
```
[点击这里看部分安装过程图片](zsh.png)

**第四步：改成极客风**

输入下面这行命令打开配置文件：

```bash
nano ~/.zshrc
```

把ZSH_THEME="robbyrussell"这句改成  

```bash
ZSH_THEME="agnoster"  
```
终端输入生效：  

```bash
source ~/.zshrc 
```

**第五步：改成二次元风格**

mkdir -p ~/.oh-my-zsh/custom/themes
cp /home/clock/.dsh/ungrouped/sakura/sakura.zsh-theme ~/.oh-my-zsh/custom/themes/
cp ~/.zshrc ~/.zshrc.bak.sakura
sed -i 's/^ZSH_THEME=.*/ZSH_THEME="sakura"/' ~/.zshrc
exec zsh

## 2.效果预览
终端上面两条是极客风，下面是二次元风，顺便调了一下透明度
[点击这里预览](yl.png)

## 3.个人知识涨进
**一、linux系统架构粗略理解**  
第一层：用户与应用程序   
第二层：Shell(命令解释器)：将用户语言翻译给内核  
第三层：内核：进程调度，内存管理，设备驱动  
第四层：硬件（cpu，gpu，内存等）  

**二、为什么要把shell中的bash换成zsh**  
1、好看，能显示箭头等等  
2、可以根据历史记录自动补全  
3、自带容错命令  
4、oh my zsh里面有很多插件可以玩  