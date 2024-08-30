#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include <time.h>
#include "mcp3004.h"

#define BASE 300
#define SPI_CHAN 0
#define X 300
#define Y 301
#define BUTTON 21
#define LED 1

#define PADDING_X 6
#define PADDING_Y 3

#define WALL "\u2588"

#define RED "\x1b[31m"
#define YELLOW "\x1b[33m"
#define DEFAULT "\x1b[0m"

struct Box{
	int x;
	int y;
	char box[3][5];
	char* color;
};

struct UserData{
	char name[5];
	char try[2];
	char time[7];
	int i_time;
	int i_try;
};

struct Node{
	struct Node* next;
	struct UserData u;
};

//function
int setup();
void gotoXY(int x, int y);
void drawTitle();
void drawMap();
int joystick();
int game();
void makeAns();
int* check(int user_ans[]);
struct Box makeBox(char c, int x, int y, char* color);
void stopwatch(int on);
void printTime(int secs, int last);
void rank(int try);
void drawResult();
void printData(struct UserData *d);
char* getName();
void drawRank();
void drawWall(){
	printf(WALL);printf(WALL);
}
void setColor(char* color){
	printf(color);
}
void drawBox(struct Box b){
	gotoXY(b.x, b.y);
	
	setColor(b.color);
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 5; j++){
			printf("%c", b.box[i][j]);
		}
		gotoXY(b.x, b.y + i + 1);
	}
	setColor(DEFAULT);
}
void setCenter(struct Box *b, char c){
	b->box[1][2] = c;
}

int x,y,b = 0;
int answer[4]={-1,-1,-1,-1};
char border[15] = { '+', ' ', '-', ' ', '+', '|', ' ', ' ', ' ', '|','+', ' ', '-', ' ', '+'};
int start_time;
int taken_time;
char char_time[7];

struct Box n1;struct Box n2;struct Box n3;
struct Box n4;struct Box n5;struct Box n6;
struct Box n7;struct Box n8;struct Box n9;
struct Box nc;struct Box n0;struct Box no;
struct Box a1;struct Box a2;struct Box a3;struct Box a4;

int main(void){
	int try;
	setup();
	try=0;
	system("clear");
	drawTitle();
	system("clear");
	try = game();
	system("clear");
	drawResult(try);
	system("clear");
	rank(try);

}

int setup(){
	if(wiringPiSetup() == -1){
		return 1;
	}
	mcp3004Setup(BASE, SPI_CHAN);
	pinMode(BUTTON, INPUT);
	pinMode(LED, OUTPUT);
	digitalWrite(LED, 0);
	srand(time(NULL));
	
	int padding_x = 6;
	int padding_y = 3;
	
	a1= makeBox('?',PADDING_X + 4,  PADDING_Y + 2, DEFAULT);
	a2= makeBox('?',PADDING_X + 12, PADDING_Y + 2, DEFAULT);
	a3= makeBox('?',PADDING_X + 20, PADDING_Y + 2, DEFAULT);
	a4= makeBox('?',PADDING_X + 28, PADDING_Y + 2, DEFAULT);

	n1= makeBox('1',PADDING_X + 40, PADDING_Y + 12, DEFAULT);
	n2= makeBox('2',PADDING_X + 48, PADDING_Y + 12, DEFAULT);
	n3= makeBox('3',PADDING_X + 56, PADDING_Y + 12, DEFAULT);
	n4= makeBox('4',PADDING_X + 40, PADDING_Y + 16, DEFAULT);
	n5= makeBox('5',PADDING_X + 48, PADDING_Y + 16, DEFAULT);
	n6= makeBox('6',PADDING_X + 56, PADDING_Y + 16, DEFAULT);
	n7= makeBox('7',PADDING_X + 40, PADDING_Y + 20, DEFAULT);
	n8= makeBox('8',PADDING_X + 48, PADDING_Y + 20, DEFAULT);
	n9= makeBox('9',PADDING_X + 56, PADDING_Y + 20, DEFAULT);
	nc= makeBox('C',PADDING_X + 40, PADDING_Y + 24, DEFAULT);
	n0= makeBox('0',PADDING_X + 48, PADDING_Y + 24, DEFAULT);
	no= makeBox('O',PADDING_X + 56, PADDING_Y + 24, DEFAULT);
	return 0;
}
void drawTitle(){
	int i;
	char* title = "BULLS AND COWS";
	char* s1 = "Press button to start...";
	
	gotoXY(PADDING_X, PADDING_Y);
	for (i = 0; i< 30; i++){
		drawWall();
	}
	for(i = 1; i < 6; i++){
		gotoXY(PADDING_X, PADDING_Y + i);
		drawWall();
		gotoXY(PADDING_X + 58, PADDING_Y + i);
		drawWall();
	}
	
	gotoXY(PADDING_X, PADDING_Y + 6);
	for (i = 0; i < 30; i++){
		drawWall();
	}
	
	gotoXY(PADDING_X + 16, PADDING_Y + 3);
	for( i = 0; i < 15 ; i++){
		printf("%c ", title[i]);
	}
	
	gotoXY(PADDING_X + 12, PADDING_Y + 9);
	printf("====================================");
	gotoXY(PADDING_X + 18, PADDING_Y + 11);
	for( i = 0; i < 24 ; i++){
		printf("%c", s1[i]);
	}
	gotoXY(PADDING_X + 12, PADDING_Y + 13);
	printf("====================================");
	
	
	
	
	gotoXY(PADDING_X + 19, PADDING_Y + 15);
	printf("UP     : joystick up");
	
	gotoXY(PADDING_X + 19, PADDING_Y + 17);
	printf("DOWN   : joystick down");
	
	gotoXY(PADDING_X + 19, PADDING_Y + 19);
	printf("RIGHT  : joystick right");
	
	gotoXY(PADDING_X + 19, PADDING_Y + 21);
	printf("LEFT   : joystick left");
	
	gotoXY(PADDING_X + 19, PADDING_Y + 23);
	printf("SELECT : press button");

	gotoXY(0, PADDING_Y + 30);
	fflush(stdout);
	
	while(1){
		b = digitalRead(BUTTON);
		if(b == 1){
			delay(300);
			break;
		}
		digitalWrite(LED, 1);
	}
	digitalWrite(LED, 0);
}
int game(){
	int n_cur[2] = {0,0};
	int a_cur = 0;
	int sel = -9;
	int num_list[12] = {1,2,3,4,5,6,7,8,9,-1,0,-2};
	char num_char_list[12] = {'1','2','3','4','5','6','7','8','9','C', '0', 'O'};
	int line = PADDING_Y + 6;
	int *sb;
	int try = 0;
	
	struct Box num_pad[4][3] = {
		{n1, n2, n3}, 
		{n4, n5, n6}, 
		{n7, n8, n9}, 
		{nc, n0, no}
		};
	struct Box ans_display[4] = {a1, a2, a3, a4};
	
	int input = -1;
	int user_ans[4] = {-1, -1, -1, -1};
	
	drawMap();
	makeAns();
	printf("ans: %d %d %d %d\n", answer[0], answer[1], answer[2], answer[3]);
	num_pad[n_cur[0]][n_cur[1]].color = YELLOW;
	drawBox(num_pad[n_cur[0]][n_cur[1]]);
	stopwatch(1);
	while(1){
		gotoXY(PADDING_X + 51, PADDING_Y + 4);
		stopwatch(2);
		input = joystick();
		//printf("joystick: %d\n", input);
		switch(input){
			case 1: 
				if(n_cur[0] > 0){
					drawBox(num_pad[n_cur[0]][n_cur[1]]);
					n_cur[0] -= 1;
					num_pad[n_cur[0]][n_cur[1]].color = YELLOW;
					drawBox(num_pad[n_cur[0]][n_cur[1]]);
				}
				break;
			case 2: 
				if(n_cur[0] < 3){
					drawBox(num_pad[n_cur[0]][n_cur[1]]);
					n_cur[0] += 1;
					num_pad[n_cur[0]][n_cur[1]].color = YELLOW;
					drawBox(num_pad[n_cur[0]][n_cur[1]]);
				}
				break;
			case 3: 
				if(n_cur[1] < 2){
					drawBox(num_pad[n_cur[0]][n_cur[1]]);
					n_cur[1] += 1;
					num_pad[n_cur[0]][n_cur[1]].color = YELLOW;
					drawBox(num_pad[n_cur[0]][n_cur[1]]);				}
				break;
			case 4: 
				if(n_cur[1] > 0){
					drawBox(num_pad[n_cur[0]][n_cur[1]]);
					n_cur[1] -= 1;
					num_pad[n_cur[0]][n_cur[1]].color = YELLOW;
					drawBox(num_pad[n_cur[0]][n_cur[1]]);
				}
				break;
			case 5: 
				sel = num_list[n_cur[0]*3 + n_cur[1]];
				//printf("sel: %d\n", sel);
				if(sel == -2){
					//printf("user_ans: %d %d %d %d\n", user_ans[0], user_ans[1], user_ans[2], user_ans[3]);
					try++;
					gotoXY(PADDING_X + 51, 9);
					printf("%d", try);
					
					gotoXY(PADDING_X + 6, line);
					for(int i = 0 ; i < 4; i++){
						printf("%d  ",user_ans[i]);
					}
					a_cur = 0;
					
					sb = check(user_ans);
					gotoXY(PADDING_X + 21, line);
					if(sb[0] == 4){
						printf("! CORRECT !");
						stopwatch(0);
						fflush(stdout);
						delay(1000);
						return try;
					}else{
						printf("%d  S  %d  B", sb[0], sb[1]);
					}
					if(line < 28){
						line++;
					}else{
						line = 9;
					}
					
					setCenter(&ans_display[0], '?');
					drawBox(ans_display[0]);
					setCenter(&ans_display[1], '?');
					drawBox(ans_display[1]);
					setCenter(&ans_display[2], '?');
					drawBox(ans_display[2]);
					setCenter(&ans_display[3], '?');
					drawBox(ans_display[3]);
					
				}else if(sel == -1){
					a_cur--;
					user_ans[a_cur] = 0;
					setCenter(&ans_display[a_cur], '?');
					drawBox(ans_display[a_cur]);
				}else{
					if(a_cur < 4){
						user_ans[a_cur] = sel;
						setCenter(&ans_display[a_cur], num_char_list[n_cur[0]*3 + n_cur[1]]);
						//printf("center: %c \n", ans_display[a_cur].box[1][2]);
						drawBox(ans_display[a_cur]);
						a_cur++;
					}
				}
				break;
			default:
				break;
		}
	fflush(stdout);
	delay(200);
	num_pad[n_cur[0]][n_cur[1]].color = DEFAULT;
	//printf("n_cur[0]: %d, n_cur[1]: %d \n", n_cur[0], n_cur[1]);
	}	
	
	
}
void stopwatch(int on){
	if(on == 1){
		start_time = (int) time(NULL);
	}else if(on == 0){
		taken_time = (int) time(NULL) - start_time;
		printTime(taken_time, 1);
	}else{
		printTime((int) time(NULL) - start_time, 0);
	}
}
void printTime(int secs, int last){
	int min, sec;
	sec = secs % 60;
	min = secs / 60 % 60;
	if(last == 0){
		printf("%02d : %02d", min, sec);
	}else{
		sprintf(char_time, "%02d : %02d", min, sec);
	}
}
void drawResult(int try){
	gotoXY(PADDING_X + 15, PADDING_Y + 6);
	printf("====================================");
	gotoXY(PADDING_X + 26, PADDING_Y + 8);
	printf(" R E S U L T ");
	gotoXY(PADDING_X + 15, PADDING_Y + 10);
	printf("====================================");
	
	gotoXY(PADDING_X + 22, PADDING_Y + 12);
	printf(" T R Y   : %d", try);
	gotoXY(PADDING_X + 22, PADDING_Y + 14);
	printf(" T I M E : %s", char_time);
	
	gotoXY(0, PADDING_Y + 30);
	
	fflush(stdout);
	
	while(1){
		b = digitalRead(BUTTON);
		if(b == 1){
			delay(500);
			break;
		}
		digitalWrite(LED, 1);
	}
	digitalWrite(LED, 0);
}
void rank(int try){
	FILE* fp = fopen("rank.txt","rb");
	struct UserData mData;
	
	int i = 0;
	int size;
	int is_empty = 1;
	
	mData.i_try = try;
	sprintf(mData.try, "%2d", mData.i_try);
	strcpy(mData.time, char_time);
	//printf("[rank] 0 mData.time: %s", mData.time);
	
	//printf("i try: %d, try: %s", mData.i_try, mData.try);
	mData.i_time = taken_time;
	
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	
	fseek(fp, 0, SEEK_SET);

	struct Node *head = malloc(sizeof(struct Node));
	struct Node *before = head;
	struct Node *cur;
	
	//printf("[rank]read");
	for(i = 0; i < (size / sizeof(mData)); i++){
			struct Node *n = malloc(sizeof(struct Node));
			fread(&n->u, sizeof(mData), 1, fp);
			before-> next = n;
			before = n;
	}
	cur = head;
	before = head;
	int in = 0;
	//printf("[rank] insert");
	if((size / sizeof(mData)) == 0){
		struct Node *n = malloc(sizeof(struct Node));
			strcpy(mData.name, getName());
			//printf("[rank]n->u.name: %s ", n->u.name);
			n->u = mData;
			cur->next = n;
			is_empty = 0;
	}else{
		for(i = 0; i < (size / sizeof(mData)); i++){
			before = cur;
			cur = cur->next;
			
			if(cur->u.i_try > mData.i_try){
				struct Node *n = malloc(sizeof(struct Node));
				strcpy(mData.name, getName());
				//printf("[rank]n->u.name: %s ", n->u.name);
				n->u = mData;
				//printf("[rank] 1 mData.time: %s, n->u.time %s", mData.time, n->u.time);
				n->next = cur;
				before->next = n;
				is_empty = 0;
				break;
			} else if(cur->u.i_try == mData.i_try && cur->u.i_time >= mData.i_time){
				struct Node *n = malloc(sizeof(struct Node));
				//printf("[rank] 2 before mData.time: %s\n", mData.time);
				strcpy(mData.name, getName());
				n->u = mData;
				//printf("[rank] 2 mData.time: %s, n->u.time %s", mData.time, n->u.time);
				n->next = cur;
				before->next = n;
				is_empty = 0;
				break;
			}
		//printf("i: %d", i);
		}
		if(i < 10 && cur->next == NULL && is_empty == 1){
				before = cur;
				cur = cur->next;
				struct Node *n = malloc(sizeof(struct Node));
				strcpy(mData.name, getName());
				n->u = mData;
				//printf("[rank] 3 mData.time: %s, n->u.time %s", mData.time, n->u.time);
				n->next = cur;
				before->next = n;
			}
	}
	//printf("[rank]out for");
	
	FILE* fpo = fopen("rank.txt","wb");
	
	cur = head;
	for(i = 0 ; i < 10 ; i++){
		if(cur->next == NULL){
			break;
		}
		cur = cur->next;
		//printf("\n[rank]write");
		fflush(stdout);
		fwrite(&cur->u, sizeof(struct UserData), 1, fpo);
		//printf("\n[rank]done");
	}
	/*
	before = head;
	for(i = 0 ; i < 10 ; i++){
		if(cur->next == NULL){
			free(cur);
			break;
		}
		before = cur;
		cur = cur -> next;
		free(before);
		printf("\n[rank]free");
	}
	*/
	fflush(stdout);
	free(head);
	fclose(fp);
	fclose(fpo);
	delay(1000);
	system("clear");
	drawRank();
	
}
void drawRank(){
	FILE* fp = fopen("rank.txt","rb");
	struct UserData mData;
	
	int size;
	
	gotoXY(PADDING_X + 15, PADDING_Y + 2);
	printf("====================================");
	gotoXY(PADDING_X + 29, PADDING_Y + 4);
	printf(" R A N K ");
	gotoXY(PADDING_X + 15, PADDING_Y + 6);
	printf("====================================");
	
	
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	//printf("\n%d %d %d", size, sizeof(mData),(size / sizeof(mData)));
	for(int i = 0 ; i < (size / sizeof(mData)); i++){
		//printf("read");
		fread(&mData, sizeof(mData), 1, fp);
		gotoXY(PADDING_X + 15, PADDING_Y + 8 + 2*i);
		printData(&mData);
	}
	
	gotoXY(0,35);
	fflush(stdout);
	
	while(1){
		b = digitalRead(BUTTON);
		if(b == 1){
			delay(300);
			break;
		}
		digitalWrite(LED, 1);
	}
	digitalWrite(LED, 0);
}

char* getName(){
	int input;
	char *name = malloc(sizeof(char) * 5);
	name[0] = 32;
	name[1] = 32;
	name[2] = 32;
	name[3] = 32;
	name[4] = 32;
	int i = 0;
	
	gotoXY(PADDING_X + 15, PADDING_Y + 6);
	printf("====================================");
	gotoXY(PADDING_X + 19, PADDING_Y + 8);
	printf(" E N T E R  T H E  N A M E ");
	gotoXY(PADDING_X + 15, PADDING_Y + 10);
	printf("====================================");
	
	gotoXY(PADDING_X + 21, PADDING_Y + 12);
	printf(" N A M E  : ");
	fflush(stdout);
	
	while(1){
		input = joystick();
		gotoXY(PADDING_X + 33, PADDING_Y + 12);
		printf("%c %c %c %c %c", name[0], name[1], name[2], name[3], name[4]);

		switch(input){
			case 1:
				if(name[i] < 90 && name[i] != 32){	
					name[i]++;
				}else if(name[i] == 32){
					name[i] = 65;
				}
				else{
					name[i] =32;
				}
				break;
			case 2:
				if(name[i] > 65){
					name[i]--;
				}else if(name[i] ==  32){
					name[i] = 90;
				}
				else{
					name[i] = 32;
				}
				break;	
			case 3:
				if(i < 4){
					i++;
				}
				break;
			case 4:
				if(i > 0){
					i--;
				}
				break;
			case 5:
				//printf("\n[getName]%c %c %c %c %c", name[0], name[1], name[2], name[3], name[4]);
				//printf("[getName]name: %s",name);
				fflush(stdout);
				delay(300);
				return name;
		}
		
		
		//gotoXY(0, 35);
		//printf("%d %c %c %c %c %c", i, name[0], name[1], name[2], name[3], name[4]);
		delay(200);
		fflush(stdout);
	}
	
	
	gotoXY(0, PADDING_Y + 30);
}
void printData(struct UserData *d){
	//printf("name: %s, try: %s, time: %s\n", d->name, d->try, d->time);
	char* name = d->name;
	printf("name: %c%c%c%c%c|", name[0], name[1], name[2], name[3], name[4]);
	printf(" try: %2d | time: %s",d->i_try, d->time);
}
int *check(int user_ans[]){
	int i,j;
	int *sb = malloc(sizeof(int) * 2);
	
	for(i=0; i<4; i++){
		for(j = 0 ; j < 4 ; j++){
			if(answer[i] == user_ans[j]){
				if(i==j) sb[0]++;
				else sb[1]++;
			}
		}
	}
	return sb;
}
void makeAns(){
	int i,j;
	int r;
	int a[4] = {-1,-1,-1,-1};
	int ok = 1;
	
	
	for(i=0; i<4; i++, ok = 1){
		r = rand()%10;		
		//printf("r: %d\n", r);
		for(j=0; j<4; j++){
			if(a[j] == r){
				i--;
				ok = -1;
				break;
			}
		}
		if(ok == 1) a[i] = r;
	}
	
	answer[0] = a[0];
	answer[1] = a[1];
	answer[2] = a[2];
	answer[3] = a[3];
	
}
void drawMap(){
	int i;
	
	gotoXY(PADDING_X, PADDING_Y);
	for (i = 0; i< 19; i++){
		drawWall();
	}
	for(i = 1; i < 27; i++){
		gotoXY(PADDING_X, PADDING_Y + i);
		drawWall();
		gotoXY(PADDING_X + 36, PADDING_Y + i);
		drawWall();
	}
	gotoXY(PADDING_X, PADDING_Y + 27);
	for (i = 0; i< 19; i++){
		drawWall();
	}
	
	drawBox(a1);
	drawBox(a2);
	drawBox(a3);
	drawBox(a4);
	
	gotoXY(PADDING_X + 18, PADDING_Y + 6);
	for(i = 0 ; i < 20; i++){
		printf("|");
		gotoXY(PADDING_X + 18, PADDING_Y + i + 7);
	}
	
	gotoXY(PADDING_X + 40, PADDING_Y + 2);
	printf("=====================");
	gotoXY(PADDING_X + 40, PADDING_Y + 4);
	printf(" T I M E : 0");
	gotoXY(PADDING_X + 40, PADDING_Y + 6);
	printf(" T R Y   : 0");
	gotoXY(PADDING_X + 40, PADDING_Y + 8);
	printf("=====================");
	
	drawBox(n1);drawBox(n2);drawBox(n3);
	drawBox(n4);drawBox(n5);drawBox(n6);
	drawBox(n7);drawBox(n8);drawBox(n9);
	drawBox(nc);drawBox(n0);drawBox(no);
	
	gotoXY(1, 35);
	
}
struct Box makeBox(char c, int x, int y, char* color){
	struct Box b;
	int i;
	for(i=0; i<15; i++){
		b.box[i/5][i%5] = border[i];
	}
	b.box[1][2] = c;
	b.x = x;
	b.y = y;
	b.color = color;
	
	return b;
}
void gotoXY(int x, int y){
	printf("\033[%dd\033[%dG",y,x);
}
int joystick(){
	x = analogRead(X);
	y = analogRead(Y);
	b = digitalRead(BUTTON);
	
	if(x > 700){
		return 1;
	}else if(x < 300){
		return 2;
	}else if(y > 680){
		return 3;
	}else if(y < 280){
		return 4;
	}else if(b == 1){
		return 5;
	}
	else{
		return 0;
	}
}
