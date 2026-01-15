#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//写的头秃



typedef struct SourseLine{
    char *source_line;
    int address;
    struct SourseLine *next;
} SourseLine;//用来存代码的

typedef struct symbol{
    char name[20];
    int address;
} symbol;//用来存标签的

typedef struct symboltable{
    symbol *content;
    int num;
    int size;
} symboltable;//用来存一堆标签的

static char* trim(char* str){
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end))
     end--;
    *(end + 1) = '\0';
    return str;
}

static void upcase_inplace(char *s){
    for(; *s; s++) *s = (char)toupper((unsigned char)*s);//把字母转成大写啊啊啊啊啊
}

static int parse_number(const char *str){
    if (!str) return 0;
    if (str[0] == 'x' || str[0] == 'X') return (int)strtol(str + 1, NULL, 16);
    if (str[0] == '#') return (int)strtol(str + 1, NULL, 10);
    return (int)strtol(str, NULL, 10);//把字符串转成数字
}

static int is_opcode_like(const char *str){
    if (!str || !*str) return 0;

    char t[64];
    snprintf(t, sizeof(t), "%s", str);
    upcase_inplace(t);

    if (!strncmp(t, "BR", 2)) return 1;

    static const char *ops[] = {
        "ADD","AND","JMP","JSR","JSRR","LD","LDI","LDR","LEA","NOT","RET","RTI",
        "ST","STI","STR","TRAP",
        "GETC","OUT","PUTS","IN","PUTSP","HALT",
        ".ORIG",".END",".FILL",".BLKW",".STRINGZ"
    };
    for (int i = 0; i < sizeof(ops)/sizeof(ops[0]); i++){
        if (!strcmp(t, ops[i])) return 1;
    }
    return 0;
}//用来判断是不是指令的

static void add_symbol(symboltable *table, const char *name, int address){
    if (!table || !name || !*name) return;
    if (table->size <= 0){
        table->size = 64;
        table->content = (symbol*)malloc((size_t)table->size * sizeof(symbol));
        table->num = 0;
    }//初始化
    if (table->num >= table->size){
        table->size *= 2;
        table->content = (symbol*)realloc(table->content, (size_t)table->size * sizeof(symbol));//数据结构里学了
    }
    snprintf(table->content[table->num].name, sizeof(table->content[table->num].name), "%s", name);
    table->content[table->num].address = address;
    table->num++;
}//把标签加到符号表里

SourseLine* Getsourse(){            //输入代码并且清洗一下
    SourseLine *head = NULL, *tail = NULL;
    char line[1000];
    char *p;
    while (fgets(line, sizeof(line), stdin)){
        line[strcspn(line, "\r\n")] = '\0';//去掉换行符 

        char *comment = strchr(line, ';');
        if (comment) *comment = '\0';

        char *trimmed = trim(line);//清除空格
        if (!trimmed || *trimmed == '\0') continue;

        SourseLine *node = (SourseLine*)malloc(sizeof(SourseLine));
        node->source_line = strdup(trimmed);
        node->address = 0;
        node->next = NULL;

        if (!head) head = tail = node;
        else { tail->next = node; tail = node; }
    }
    return head;
}

void firstpass(symboltable *table, SourseLine *head){
    int pc = 0;
    char pending[64][20];
    int pending_n = 0;

    for (SourseLine *p = head; p; p = p->next){//遍历每一行代码
        char linebuf[1000];
        snprintf(linebuf, sizeof(linebuf), "%s", p->source_line);

        char tmp[1000];
        snprintf(tmp, sizeof(tmp), "%s", linebuf);

        char *save = NULL;
        char *tok = strtok_r(tmp, " \t,", &save);//把每行代码第一个单词给挑出来，下面用来判断
        if (!tok) continue;

        char labels[64][20];
        int label_num = 0;

        while (tok && !is_opcode_like(tok)){//token不是指令的话，那就是标签
            if (label_num < 64){
                snprintf(labels[label_num], sizeof(labels[label_num]), "%s", tok);
                upcase_inplace(labels[label_num]);
                label_num++;
            }//把标签存起来
            tok = strtok_r(NULL, " \t,", &save);
        }

        if (!tok){//没有指令的话
            for (int i = 0; i < label_num && pending_n < 64; i++){
                snprintf(pending[pending_n], sizeof(pending[pending_n]), "%s", labels[i]);//把标签存到待定列表里
                pending_n++;
            }
            p->address = pc;//地址存到节点里
            continue;
        }//没有指令的话就继续下一行

        char op[64];//操作码
        snprintf(op, sizeof(op), "%s", tok);
        upcase_inplace(op);//把操作码转成大写

        if (!strcmp(op, ".ORIG")){
            char *imm = strtok_r(NULL, " \t,", &save);//取出地址，沿着上次切的线继续切
            pc = parse_number(imm);
            p->address = pc;
            for (int i = 0; i < pending_n; i++) add_symbol(table, pending[i], pc);
            pending_n = 0;
            for (int i = 0; i < label_num; i++) add_symbol(table, labels[i], pc);
            continue;
        }

        p->address = pc;

        for (int i = 0; i < pending_n; i++) add_symbol(table, pending[i], pc);
        pending_n = 0;
        for (int i = 0; i < label_num; i++) add_symbol(table, labels[i], pc);

        if (!strcmp(op, ".END")) break;

        int pc_delta = 1;

        if (!strcmp(op, ".BLKW")){
            char *cnt = strtok_r(NULL, " \t,", &save);
             pc_delta = parse_number(cnt);
            if (pc_delta < 0) pc_delta = 0;
        } else if (!strcmp(op, ".STRINGZ")){
            char *q1 = strchr(linebuf, '"');//找到第一个引号
            char *q2 = strrchr(linebuf, '"');//找到最后一个引号
            int len = 0;
            if (q1 && q2 && q2 > q1){
                for (char *s = q1 + 1; s < q2; ){
                    if (*s == '\\' && (s + 1) < q2 && s[1] == 'n'){
                        len++;
                        s += 2;
                    } else {
                        len++;
                        s++;
                    }
                }
            }
            pc_delta = len + 1;
        } else if (!strcmp(op, ".FILL")){
            pc_delta = 1;
        } else {
            pc_delta = 1;
        }

        pc += pc_delta;
    }//for循环结束，处理了所有行
}
//firstpass结束，符号表建好了，地址也算好了

static void Printf16(unsigned int word){
    for (int k = 15; k >= 0; k--){
        putchar(((word >> k) & 1U) ? '1' : '0');
    }
    putchar('\n');
}

static void Remove_douhao(char *s){
    size_t n = strlen(s);
    if (n > 0 && s[n - 1] == ',') s[n - 1] = '\0';
}//去掉逗号

static int parse_number_token(const char *token){
    if (!token) return 0;
    char buf[64];
    snprintf(buf, sizeof(buf), "%s", token);
    Remove_douhao(buf);
    return parse_number(buf);
}

static int RegNum(const char *token){
    if (!token) return -1;
    char buf[64];
    snprintf(buf, sizeof(buf), "%s", token);
    Remove_douhao(buf);
    if (buf[0] != 'R' && buf[0] != 'r') return -1;
    int v = (int)strtol(buf + 1, NULL, 10);
    if (v < 0 || v > 7) return -1;
    return v;
}//把寄存器的字符串转成数字

static int IsReg(const char *token){
    return RegNum(token) >= 0;
}//判断是不是寄存器

static int lookup_symbol(symboltable *table, const char *name, int *outaddr){
    if (!table || !name) return 0;
    char key[64];
    snprintf(key, sizeof(key), "%s", name);
    Remove_douhao(key);
    upcase_inplace(key);
    for (int i = 0; i < table->num; i++){
        if (!strcmp(table->content[i].name, key)){
            if (outaddr) *outaddr = table->content[i].address;
            return 1;
        }
    }
    return 0;
}//在符号表里查找标签

static int is_number_token(const char *token);

static int resolve_value(symboltable *table, const char *token){
    if (!token) return 0;
    char buf[64];//缓冲
    snprintf(buf, sizeof(buf), "%s", token);
    Remove_douhao(buf);
    if (is_number_token(buf)){
        return parse_number(buf);
    }
    int addr = 0;
    if (lookup_symbol(table, buf, &addr)) return addr;
    return 0;
}//解析值，要么是数字，要么是标签，解析操作数

static int is_number_token(const char *token){
    if (!token || !*token) return 0;
    const char *p = token;
    if (*p == '#') p++;
    if (*p == '-') p++;
    if (*p == 'x' || *p == 'X'){
        p++;
        if (*p == '-') p++;
        if (!isxdigit((unsigned char)*p)) return 0;
        while (*p){
            if (!isxdigit((unsigned char)*p)) return 0;
            p++;
        }
        return 1;
    }
    if (!isdigit((unsigned char)*p)) return 0;
    while (*p){
        if (!isdigit((unsigned char)*p)) return 0;
        p++;
    }
    return 1;
}

static int pc_offset(symboltable *table, const char *target, int instr_addr, int width){
    int target_addr = resolve_value(table, target);
    int off = target_addr - (instr_addr + 1);
    int mask = (1 << width) - 1;
    return off & mask;
}

static int trap_alias_vec(const char *op){
    if (!strcmp(op, "GETC")) return 0x20;
    if (!strcmp(op, "OUT")) return 0x21;
    if (!strcmp(op, "PUTS")) return 0x22;
    if (!strcmp(op, "IN")) return 0x23;
    if (!strcmp(op, "PUTSP")) return 0x24;
    if (!strcmp(op, "HALT")) return 0x25;
    return -1;
}

void secondpass(symboltable *table, SourseLine *head){
    int origin_set = 0;
    int origin = 0;

    for (SourseLine *p = head; p; p = p->next){
        char linebuf[1000];
        snprintf(linebuf, sizeof(linebuf), "%s", p->source_line);
        if (linebuf[0] == '\0') continue;//句子为空就跳过

        char tmp[1000];
        snprintf(tmp, sizeof(tmp), "%s", linebuf);
        char *save = NULL;
        char *tok = strtok_r(tmp, " \t,", &save);
        if (!tok) continue;

        while (tok && !is_opcode_like(tok)){
            tok = strtok_r(NULL, " \t,", &save);
        }
        if (!tok) continue;

        char op[64];
        snprintf(op, sizeof(op), "%s", tok);//取第一个单词
        upcase_inplace(op);

        if (!strcmp(op, ".ORIG")){
            char *m = strtok_r(NULL, " \t,", &save);
            origin = parse_number_token(m);
            origin_set = 1;
            Printf16((unsigned int)origin & 0xFFFFU);
            continue;
        }

        if (!origin_set) continue;
        if (!strcmp(op, ".END")) break;

        int instr_addr = p->address;

        if (!strcmp(op, ".FILL")){
            char *vTok = strtok_r(NULL, " \t,", &save);
            int val = resolve_value(table, vTok);
            Printf16((unsigned int)val & 0xFFFFU);
            continue;
        }

        if (!strcmp(op, ".BLKW")){
            char *nTok = strtok_r(NULL, " \t,", &save);
            int n = parse_number_token(nTok);
            if (n < 0) n = 0;
            for (int i = 0; i < n; i++) Printf16(0);
            continue;
        }

        if (!strcmp(op, ".STRINGZ")){
            char *q1 = strchr(linebuf, '"');//找到第一个引号
            char *q2 = strrchr(linebuf, '"');//找到最后一个引号
            if (!q1 || !q2 || q2 <= q1){
                Printf16(0);//字符串为空
                continue;
            }
            for (char *s = q1 + 1; s < q2; ){
                if (*s == '\\' && (s + 1) < q2 && s[1] == 'n'){
                    Printf16(10);//换行符
                    s += 2;
                } else {
                    Printf16((unsigned int)(unsigned char)*s);
                    s++;
                }
            }
            Printf16(0);
            continue;
        }

        int vec = trap_alias_vec(op);
        if (vec != -1){
            Printf16(0xF000U | (unsigned int)(vec & 0xFF));
            continue;
        }

        if (!strcmp(op, "TRAP")){
            char *vTok = strtok_r(NULL, " \t,", &save);
            int v = parse_number_token(vTok) & 0xFF;
            Printf16(0xF000U | (unsigned int)v);
            continue;
        }

        if (!strcmp(op, "ADD") || !strcmp(op, "AND")){
            char *drreg = strtok_r(NULL, " \t,", &save);//目的寄存器
            char *sr1reg = strtok_r(NULL, " \t,", &save);
            char *xTok = strtok_r(NULL, " \t,", &save);//第二个源寄存器或者立即数
            int DR = RegNum(drreg);
            int SR1 = RegNum(sr1reg);
            int opcode = (!strcmp(op, "ADD")) ? 0x1 : 0x5;
            if (IsReg(xTok)){
                int SR2 = RegNum(xTok);
                unsigned int word = (opcode << 12) | (DR << 9) | (SR1 << 6) | SR2;//第5位是0
                Printf16(word);
            } else {
                int imm5 = parse_number_token(xTok) & 0x1F;
                unsigned int word = (opcode << 12) | (DR << 9) | (SR1 << 6) | (1 << 5) | imm5;
                Printf16(word);
            }
            continue;
        }

        if (!strcmp(op, "NOT")){
            char *drTok = strtok_r(NULL, " \t,", &save);
            char *srTok = strtok_r(NULL, " \t,", &save);
            int DR = RegNum(drTok);
            int SR = RegNum(srTok);
            unsigned int word = 0x9000U | (DR << 9) | (SR << 6) | 0x003FU;
            Printf16(word);
            continue;
        }

        if (!strncmp(op, "BR", 2)){
            int n = 0, z = 0, pbit = 0;
            if (strlen(op) == 2){
                n = z = pbit = 1;
            } else {
                for (const char *ch = op + 2; *ch; ch++){
                    if (*ch == 'N') n = 1;
                    if (*ch == 'Z') z = 1;
                    if (*ch == 'P') pbit = 1;
                }
            }
            char *labelTok = strtok_r(NULL, " \t,", &save);
            int off9 = pc_offset(table, labelTok, instr_addr, 9);
            unsigned int word = (n << 11) | (z << 10) | (pbit << 9) | (unsigned int)off9;
            Printf16(word);
            continue;
        }

        if (!strcmp(op, "RET")){
            unsigned int word = 0xC000U | (7 << 6);
            Printf16(word);
            continue;
        }

        if (!strcmp(op, "JMP")){
            char *baseTok = strtok_r(NULL, " \t,", &save);
            int BaseR = RegNum(baseTok);
            unsigned int word = 0xC000U | (BaseR << 6);
            Printf16(word);
            continue;
        }

        if (!strcmp(op, "JSR")){
            char *labelTok = strtok_r(NULL, " \t,", &save);
            int off11 = pc_offset(table, labelTok, instr_addr, 11);
            unsigned int word = 0x4800U | (unsigned int)off11;
            Printf16(word);
            continue;
        }

        if (!strcmp(op, "JSRR")){
            char *baseTok = strtok_r(NULL, " \t,", &save);
            int BaseR = RegNum(baseTok);
            unsigned int word = 0x4000U | (BaseR << 6);
            Printf16(word);
            continue;
        }

        if (!strcmp(op, "LD") || !strcmp(op, "LDI") || !strcmp(op, "LEA") ||
            !strcmp(op, "ST") || !strcmp(op, "STI")){
            char *rTok = strtok_r(NULL, " \t,", &save);
            char *labelTok = strtok_r(NULL, " \t,", &save);
            int off9 = pc_offset(table, labelTok, instr_addr, 9);
            int opcode = 0;
            if (!strcmp(op, "LD")) opcode = 0x2;
            else if (!strcmp(op, "ST")) opcode = 0x3;
            else if (!strcmp(op, "LDI")) opcode = 0xA;
            else if (!strcmp(op, "STI")) opcode = 0xB;
            else if (!strcmp(op, "LEA")) opcode = 0xE;
            int R = RegNum(rTok);
            unsigned int word = (opcode << 12) | (R << 9) | (unsigned int)off9;
            Printf16(word);
            continue;
        }

        if (!strcmp(op, "LDR") || !strcmp(op, "STR")){
            char *rTok = strtok_r(NULL, " \t,", &save);
            char *baseTok = strtok_r(NULL, " \t,", &save);
            char *offTok = strtok_r(NULL, " \t,", &save);
            int R = RegNum(rTok);
            int BaseR = RegNum(baseTok);
            int off6 = parse_number_token(offTok) & 0x3F;
            int opcode = (!strcmp(op, "LDR")) ? 0x6 : 0x7;
            unsigned int word = (opcode << 12) | (R << 9) | (BaseR << 6) | (unsigned int)off6;
            Printf16(word);
            continue;
        }

        if (!strcmp(op, "RTI")){
            Printf16(0x8000U);
            continue;
        }

        if (!strcmp(op, "RESERVED")){
            Printf16(0xD000U);
            continue;
        }//if结束，穷举所有操作数
    }//for结束
}

int main(void){
    SourseLine *head = Getsourse();
    symboltable table;
    memset(&table, 0, sizeof(table));

    firstpass(&table, head);
    secondpass(&table, head);
    return 0;
}
