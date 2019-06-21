#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"




#define BUF_SIZE 256
#define MAX_LINE_NUMBER 256
#define MAX_LINE_LENGTH 256
#define NULL 0

char* strcat_n(char* dest, char* src, int len);
int get_line_number(char *text[]);
void show_welcome();
void show_text(char *text[]);
void com_ins(char *text[], int n, char *extra);
void com_mod(char *text[], int n, char *extra);
void com_del(char *text[], int n);
void com_help(char *text[]);
void com_save(char *text[], char *path);
void com_exit(char *text[], char *path);
int stringtonumber(char* src);
void swap(char *text[], int n, char *extra);

//标记是否更改过
int changed = 0;
int auto_show = 1;

//文件名
char *p;

//删除完后要显示全文，1要，0不要（因为swap调用了删除，故设此变量）
int swap_flag = 1;

int new_flag = 0;
char *name;

int main(int argc, char *argv[])
{
	//setProgramStatus(EDITOR);
	if (argc == 1)
	{
		printf(1, "指令输入格式为 [editor file_name]\n");
		//setProgramStatus(SHELL);
		exit();
	}
	//存放文件内容
	
	char *text[MAX_LINE_NUMBER] = {};
	text[0] = malloc(MAX_LINE_LENGTH);
	memset(text[0], 0, MAX_LINE_LENGTH);
	//存储当前最大的行号，从0开始。即若line_number == x，则从text[0]到text[x]可用
	int line_number = 0;
	//尝试打开文件
	int fd = open(argv[1], O_RDONLY);
	
	// 如果文件不存在，创建一个新文件，确保文件创建成功
	if(fd == -1)
	{
		if (mkdir(argv[1]) < 0) {
			printf(2, "\033[35;32m文件%s创建失败！！！文件已存在？\033[0m \n", argv);
			exit();
		} else {
			new_flag = 1;
			name = argv[1];
			printf(1, "新文件！ %s\n", argv[1]);
		}
	}
	fd = open(argv[1], O_RDONLY);
	
	//如果文件存在，则打开并读取里面的内容
	if (fd != -1)
	{
		printf(1, "文件存在\n");
		p = argv[1];//保存文件名为全局变量
		char buf[BUF_SIZE] = {};
		int len = 0;
		while ((len = read(fd, buf, BUF_SIZE)) > 0)
		{
			int i = 0;
			int next = 0;
			int is_full = 0;
			while (i < len)
			{
				//拷贝"\n"之前的内容
				for (i = next; i < len && buf[i] != '\n'; i++);
				strcat_n(text[line_number], buf+next, i-next);
				//必要时新建一行
				if (i < len && buf[i] == '\n')
				{
					if (line_number >= MAX_LINE_NUMBER - 1)
						is_full = 1;
					else
					{
						line_number++;
						text[line_number] = malloc(MAX_LINE_LENGTH);
						memset(text[line_number], 0, MAX_LINE_LENGTH);
					}
				}
				if (is_full == 1 || i >= len - 1)
					break;
				else
					next = i + 1;
			}
			if (is_full == 1)
				break;
		}
		close(fd);
	} else {
		printf(1, "文件打开失败！！！\n");
		exit();
	}
    show_welcome();    
    	
	//输出文件内容
	show_text(text);
	//输出帮助
	com_help(text);
	
	//处理命令
	char input[MAX_LINE_LENGTH] = {};
	while (1)
	{
		printf(1, "\n请输入命令:\n");
		memset(input, 0, MAX_LINE_LENGTH);
		gets(input, MAX_LINE_LENGTH);
		int len = strlen(input);
		input[len-1] = '\0';
		len --;
		//寻找命令中第一个空格
		int pos = MAX_LINE_LENGTH - 1;
		int j = 0;
		for (; j < 8; j++)
		{
			if (input[j] == ' ')
			{
				pos = j + 1;
				break;
			}
		}
		//ins
		if (input[0] == 'i' && input[1] == 'n' && input[2] == 's')
		{
			if (input[3] == '-'&&stringtonumber(&input[4])>=0)
			{
				com_ins(text, stringtonumber(&input[4]), &input[pos]);
                                 //插入操作需要更新行号
				line_number = get_line_number(text);
			}
			else if(input[3] == ' '||input[3] == '\0')
			{
				com_ins(text, line_number+1, &input[pos]);
                                line_number = get_line_number(text);
			}
			else
			{
				printf(2, "\033[35;32m非法输入！！！\033[0m \n");
				com_help(text);
			}
		}
		//mod
		else if (input[0] == 'm' && input[1] == 'o' && input[2] == 'd')
		{
			if (input[3] == '-'&&stringtonumber(&input[4])>=0)
				com_mod(text, atoi(&input[4]), &input[pos]);
			else if(input[3] == ' '||input[3] == '\0')
				com_mod(text, line_number + 1, &input[pos]);
			else
			{
				printf(2, "\033[35;32m非法输入！！！\033[0m \n");
				com_help(text);
			}
		}
		//del
		else if (input[0] == 'd' && input[1] == 'e' && input[2] == 'l')
		{
			if (input[3] == '-'&&stringtonumber(&input[4])>=0)
			{
				com_del(text, atoi(&input[4]));
                                //删除操作需要更新行号
				line_number = get_line_number(text);
			}	
			else if(input[3]=='\0')
			{
				com_del(text, line_number + 1);
				line_number = get_line_number(text);
			}
			else
			{
				printf(2, "\033[35;32m非法输入！！！\033[0m \n");
				com_help(text);
			}
			
		}
		//show
		else if (strcmp(input, "show") == 0)
		{
			auto_show = 1;
			printf(1, "已开启在文本更改后显示当前内容。\n");
		}
		//hide
		else if (strcmp(input, "hide") == 0)
		{
			auto_show = 0;
			printf(1, "已禁用在文本更改后显示当前内容。\n");
		}
		//help
		else if (strcmp(input, "help") == 0)
			com_help(text);
		//save
		else if (strcmp(input, "save") == 0 || strcmp(input, "CTRL+S\n") == 0)
			com_save(text, argv[1]);
		//exit
		else if (strcmp(input, "exit") == 0)
			com_exit(text, argv[1]);
		//swap-*** ***
		else if (input[0] == 's' && input[1] == 'w' && input[2] == 'a' && input[3] == 'p')
		{
			if (input[4] == '-' && stringtonumber(&input[5])>= 0)
				swap(text, atoi(&input[5]), &input[pos]);
			else
			{
				printf(2, "\033[35;32m非法输入！！！\033[0m \n");
				com_help(text);
			}
		}
		
		else
		{
			printf(2, "\033[35;32m非法输入！！！\033[0m \n");
			com_help(text);
		}
	}
	//setProgramStatus(SHELL);
	
	exit();
}

//拼接src的前n个字符到dest
char* strcat_n(char* dest, char* src, int len)
{
	if (len <= 0)
		return dest;
	int pos = strlen(dest);
	if (len + pos >= MAX_LINE_LENGTH)
		return dest;
	int i = 0;
	for (; i < len; i++)
		dest[i+pos] = src[i];
	dest[len+pos] = '\0';
	return dest;
}
void show_welcome()
{ 
	printf(1,"*******************************************************************************************\n");
    printf(1,"\033[43;35m  *     *      *   *******   *           ****       ****           *       *      *******   \033[0m \n");
	printf(1,"\033[43;35m   *   *  *   *    *         *          *          *     *        * *     * *     *         \033[0m \n");
	printf(1,"\033[43;35m    *  *   * *     *****     *         *          *       *      *   *   *   *    ****      \033[0m \n");
	printf(1,"\033[43;35m     **     **     *         *          *          *     *      *     * *     *   *         \033[0m \n");
	printf(1,"\033[43;35m      *     *      *******   ********    *****      *****      *       *       *  *******   \033[0m \n");
	printf(1,"********************************************************************************************\n");

}
void show_text(char *text[])
{
	printf(1, "\033[40;32m文件【%s】:\033[0m \n", p);
	int j = 0;
	for (; text[j] != NULL; j++)
		printf(1, "%d%d%d:%s\n", (j+1)/100, ((j+1)%100)/10, (j+1)%10, text[j]);
}


//获取当前最大的行号，从0开始，即return x表示text[0]到text[x]可用
int get_line_number(char *text[])
{
	int i = 0;
	for (; i < MAX_LINE_NUMBER; i++)
		if (text[i] == NULL)
			return i - 1;
	return i - 1;
}

int stringtonumber(char* src)
{
	int number = 0; 
	int i=0;
	int pos = strlen(src);
	for(;i<pos;i++)
	{
		if(src[i]==' ') break;
		if(src[i]>57||src[i]<48) return -1;
		number=10*number+(src[i]-48);
	}
	return number;
}

//插入命令，n为用户输入的行号，从1开始
//extra:输入命令时接着的信息，代表待插入的文本
void com_ins(char *text[], int n, char *extra)
{
	if (n < 0 || n > get_line_number(text) + 1)
	{
		printf(2, "\033[35;32m非法行号！！！\033[0m \n");
		return;
	}
	char input[MAX_LINE_LENGTH] = {};
	if (*extra == '\0')
	{
		printf(1, "请输入内容:\n");
		gets(input, MAX_LINE_LENGTH);
		input[strlen(input)-1] = '\0';
	}
	else
		strcpy(input, extra);
	int i = MAX_LINE_NUMBER - 1;
	for (; i > n; i--)
	{
		if (text[i-1] == NULL)
			continue;
		else if (text[i] == NULL && text[i-1] != NULL)
		{
			text[i] = malloc(MAX_LINE_LENGTH);
			memset(text[i], 0, MAX_LINE_LENGTH);
			strcpy(text[i], text[i-1]);
		}
		else if (text[i] != NULL && text[i-1] != NULL)
		{
			memset(text[i], 0, MAX_LINE_LENGTH);
			strcpy(text[i], text[i-1]);
		}
	}
	if (text[n] == NULL)
	{
		text[n] = malloc(MAX_LINE_LENGTH);
		if (text[n-1][0] == '\0')
		{
			memset(text[n], 0, MAX_LINE_LENGTH);
			strcpy(text[n-1], input);
			changed = 1;
			if (auto_show == 1)
				show_text(text);
			return;
		}
	}
	memset(text[n], 0, MAX_LINE_LENGTH);
	strcpy(text[n], input);
	changed = 1;
	if (auto_show == 1 && swap_flag == 1)
		show_text(text);
}

//修改命令，n为用户输入的行号，从1开始
//extra:输入命令时接着的信息，代表待修改成的文本
void com_mod(char *text[], int n, char *extra)
{
	if (n <= 0 || n > get_line_number(text) + 1)
	{
		printf(1, "非法的行号。\n");
		return;
	}
	char input[MAX_LINE_LENGTH] = {};
	if (*extra == '\0')
	{
		printf(1, "请输入内容:\n");
		gets(input, MAX_LINE_LENGTH);
		input[strlen(input)-1] = '\0';
	}
	else
		strcpy(input, extra);
	memset(text[n-1], 0, MAX_LINE_LENGTH);
	strcpy(text[n-1], input);
	changed = 1;
	if (auto_show == 1)
		show_text(text);
}
//调换命令
void swap(char *text[], int n, char *extra)
{

	/*printf(1, "%d\n", n);
	printf(1, "%s\n", text[n - 1]);
	printf(1, "%d\n", atoi(extra));
	printf(1, "%s\n", text[atoi(extra) - 1]);*/
	char *swap_a = malloc(MAX_LINE_LENGTH);
	memset(swap_a, 0, MAX_LINE_LENGTH);
	char *swap_b = malloc(MAX_LINE_LENGTH);
	memset(swap_b, 0, MAX_LINE_LENGTH);


	//输入非法
	if (n <= 0 || n > get_line_number(text) + 1)
	{
		printf(1, "非法的行号。\n");
		return;
	}
	char input[MAX_LINE_LENGTH] = {};
	//后面的行数没输入
	if (*extra == '\0')
	{
		printf(1, "请输入需要对调的行");
		gets(input, MAX_LINE_LENGTH);
		input[strlen(input) - 1] = '\0';
	}
	else
		strcpy(input, extra);

	int m = atoi(input);
	strcpy(swap_a, text[n - 1]);
	strcpy(swap_b, text[m - 1]);

	//删除调换的两行
	swap_flag = 0;
	com_del(text, m);
	com_del(text, n);

	//比较m，n大小，大的先插入，防止序号出错
	if (n > m)
	{
		com_ins(text, m - 1, swap_a);
		com_ins(text, n - 1, swap_b);
	}
	else
	{
		com_ins(text, n - 1, swap_b);
		com_ins(text, m - 1, swap_a);
	}

	swap_flag = 1; //恢复删除，插入后显示
	changed = 1;
	if (auto_show == 1 && swap_flag ==1)
		show_text(text);

}

//删除命令，n为用户输入的行号，从1开始
void com_del(char *text[], int n)
{
	if (n <= 0 || n > get_line_number(text) + 1)
	{
		printf(1, "非法的行号。\n");
		return;
	}
	memset(text[n-1], 0, MAX_LINE_LENGTH);
	int i = n - 1;
	for (; text[i+1] != NULL; i++)
	{
		strcpy(text[i], text[i+1]);
		memset(text[i+1], 0, MAX_LINE_LENGTH);
	}
	if (i != 0)
	{
		free(text[i]);
		text[i] = 0;
	}
	changed = 1;
	if (auto_show == 1 && swap_flag == 1)
		show_text(text);
}

void com_help(char *text[])
{
	printf(1, "\033[40;31m ******************************************\033[0m \n");
	printf(1, "\033[40;31m *    指令说明:                           *\033[0m \n");
	printf(1, "\033[40;31m *    ins-n, 在第n行后插入一行            *\033[0m \n");
	printf(1, "\033[40;31m *    mod-n, 修改第n行                    *\033[0m \n");
	printf(1, "\033[40;31m *    del-n, 删除第n行                    *\033[0m \n");
	printf(1, "\033[40;31m *    ins, 在最后一行后插入一行           *\033[0m \n");
	printf(1, "\033[40;31m *    mod, 修改最后一行                   *\033[0m \n");
	printf(1, "\033[40;31m *    del, 删除最后一行                   *\033[0m \n");
	printf(1, "\033[40;31m *    show, 开启功能：文字回显            *\033[0m \n");
	printf(1, "\033[40;31m *    hide, 关闭功能：文字回显            *\033[0m \n");
	printf(1, "\033[40;31m *    save, 保存文件                      *\033[0m \n");
	printf(1, "\033[40;31m *    exit, 退出编辑器                    *\033[0m \n");
	printf(1, "\033[40;31m *    help, 显示帮助                      *\033[0m \n");
	printf(1, "\033[40;31m *    swap-n m 交换n与m行内容             *\033[0m \n");
	printf(1, "\033[40;31m ******************************************\033[0m \n");
}

void com_save(char *text[], char *path)
{
	//删除旧有文件
	unlink(path);
	//新建文件并打开
	int fd = open(path, O_WRONLY|O_CREATE);
	if (fd == -1)
	{
		printf(1, "保存失败，文件无法打开:\n");
		//setProgramStatus(SHELL);
		exit();
	}
	if (text[0] == NULL)
	{
		close(fd);
		return;
	}
	//写数据
	write(fd, text[0], strlen(text[0]));
	int i = 1;
	for (; text[i] != NULL; i++)
	{
		printf(fd, "\n");
		write(fd, text[i], strlen(text[i]));
	}
	close(fd);
	new_flag = 0;
	printf(1, "\033[40;32m保存成功！！！\033[0m \n");
	changed = 0;
	return;
}

void com_exit(char *text[], char *path)
{
	//询问是否保存
	while (changed == 1 || new_flag == 1)
	{
		printf(1, "保存文件? y/n\n");
		char input[MAX_LINE_LENGTH] = {};
		gets(input, MAX_LINE_LENGTH);
		input[strlen(input)-1] = '\0';
		if (strcmp(input, "y") == 0) {
			com_save(text, path);
		} else if(strcmp(input, "n") == 0) {
			if(new_flag == 1) {
				unlink(name);
			}
			break;
		} else
		printf(2, "输入错误?\n");
	}
	//释放内存
	int i = 0;
	for (; text[i] != NULL; i++)
	{
		free(text[i]);
		text[i] = 0;
	}
	//退出
	//setProgramStatus(SHELL);
	exit();
}



