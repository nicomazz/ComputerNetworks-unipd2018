#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>  // Per il comando write
#include <time.h>
#include <string.h>
#include <time.h>
// Prototipi di funzioni
unsigned short myhtons(unsigned short s);
unsigned char *myhtonx(unsigned char *s, unsigned char *o, int size);
long hexToLong(char *str);

// Indirizzo IPv4
struct sockaddr_in indirizzo;

//Struttura singono header (nome, valore)
struct header {
	char *n;
	char *v;
};
struct header h[100];  //Header ricevuto dalla Full-Response (array di strutture)
char file_name_buff[1000];
//return d1 < d2
char date_buf[1000];

char* getNowHttpDate(){
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(date_buf, sizeof date_buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
	printf("Time is: [%s]\n", date_buf);
	return date_buf;
}
time_t httpTimeToEpoch(char * time){
	struct tm tm;
	char buf[255];
	memset(&tm, 0, sizeof(struct tm));
	strptime(time,"%a, %d %b %Y %H:%M:%S %Z", &tm);
	return mktime(&tm);
}

unsigned char compareHttpDates(char * d1, char * d2){
	return httpTimeToEpoch(d1) < httpTimeToEpoch(d2);
}
char * uriToCachedFile(char * uri){
	sprintf(file_name_buff,"./cache/%s",uri);
	return file_name_buff;
}
unsigned char expired(char * uri, char * last_modified){
	char * complete_name = uriToCachedFile(uri);
	FILE * fp = fopen(complete_name,"r");
	if (fp == NULL) return 1;
	char * line = 0; size_t len = 0;
	getline(&line,&len,fp);
	if (compareHttpDates(last_modified,line)) return 0;
	return 1;
	//todo read First line and compare
}
void printFile(char * uri){
	char * file_name = uriToCachedFile(uri);
	FILE * fp = fopen(file_name, "r");
	if (fp == NULL){
		printf("It was not present!\n");
		exit(EXIT_FAILURE);
	}
	char * line= 0;
	size_t n = 0;
	ssize_t nread;
	char first = 1;
	while ((nread = getline(&line, &n, fp)) != -1) {
		if (first) {first = 0; continue;}
		//		printf("Retrieved line of length %zu:\n", read);
		printf("%s", line);
	}

	fclose(fp);
} 
void saveToCache(char * uri,char * body){
	char * file_name = uriToCachedFile(uri);
	FILE * fp = fopen(file_name, "w");
	if (fp == NULL) 
		exit(EXIT_FAILURE);
	char * now_date = getNowHttpDate();
	while (*now_date) fputc(*(now_date++),fp);
	fputc('\n',fp);
	while (*body) fputc(*(body++),fp);
	printf("CACHED WITH SUCCESS!\n");
}
int main() {
	long dim;
	int s, k, t, i, j;  //s:file descriptor socket per inviare
	char *p;
	char primiduepunti, chunked = 0;         //chunked indica se l'header Transfer-Encodind=chunked
	char *status_line, *entity, *chunksize;  // Puntatori a: StatusLine, EntityBody, ChunkSize
	int content_length;                      //Valore che verrà reperito dall'header Content-Length
	char request[1000];
	char response[100000];
	unsigned short u, v;
	u = 0xABCD;
	myhtonx((unsigned char *)&u, (unsigned char *)&v, sizeof(unsigned short));
	printf("%X %X\n", u, v);

	//APRIRE COMUNICAZIONE
	/*socket restituisce un INT che è un file descriptor
	  ovvero l'indice della tabella con tutto ciò che serve per gestire la comunicazione*/
	s = socket(AF_INET, SOCK_STREAM, 0);  //AF_INET=IPv4, SOCK_STREAM:apre uno stream, protocol:0 (default)
	if (s == -1) {
		perror("Socket Fallita");
	}

	//CONNESSIONE
	indirizzo.sin_family = AF_INET;  //IPv4
	indirizzo.sin_port = htons(80);  //Porta80
	((unsigned char *)&(indirizzo.sin_addr.s_addr))[0] = 93;
	((unsigned char *)&(indirizzo.sin_addr.s_addr))[1] = 184;
	((unsigned char *)&(indirizzo.sin_addr.s_addr))[2] = 216;
	((unsigned char *)&(indirizzo.sin_addr.s_addr))[3] = 3;
	// google.com 216.58.213.228
	// google.co.uk 74.125.206.94
	// unipd.it 147.162.235.155
	// example.com 93.184.216.3
	t = connect(s, (struct sockaddr *)&indirizzo, sizeof(struct sockaddr_in));
	if (t == -1) perror("Connect fallita\n");

	//RICHIESTA
	/*Full-Request = Request-Line
	 *( General-Header
	 | Request-Header
	 | Entity-Header )
	 CRLF
	 [ Entity-Body ]   */
	// Request-Line = Method SP Request-URI SP HTTP-Version CRLF   (con CRLF=\r\n)
	// Header = nome: valore

	// Preparo la richiesta: (scrivo sull'array di char request)
	const char * URI = "/45/abc";
	sprintf(request, "GET %s HTTP/1.1\r\nHost:www.example.com\r\n\r\n",URI);
	//sprintf(request,"GET / HTTP/1.1\r\nHost:www.google.com\r\n\r\n");

	// Invio la richiesta:
	write(s, request, strlen(request));  // Scrivo sul file descriptor s del socket

	//RICEVO LA FULL-RESPONSE
	/*Full-Response = Status-Line
	 *( General-Header
	 | Response-Header
	 | Entity-Header )
	 CRLF
	 [ Entity-Body ]   */
	// Status-Line = HTTP-version SP Status-Code SP Reason-Phrase CRLF
	// Header = nome: valore

	// Status Line:
	h[0].n = response;
	status_line = h[0].n;
	h[0].v = h[0].n;

	//Headers
	for (i = 0, j = 0; read(s, response + i, 1); i++) {
		if ((i > 1) && (response[i] == '\n') && (response[i - 1] == '\r')) {
			primiduepunti = 1;
			response[i - 1] = 0;
			if (h[j].n[0] == 0) break;
			h[++j].n = response + i + 1;
		}
		if (primiduepunti && (response[i] == ':')) {
			h[j].v = response + i + 1;
			response[i] = 0;
			primiduepunti = 0;
		}
	}

	//Visualizzo a schermo La StatusLine e gli Headers ricevuti
	printf("Status Line: %s\n", status_line);

	// di interesse per ciò che dobbiamo fare
	char * last_modified = 0;
	char uri[100];
	unsigned char body[100000];
	strcpy(uri,URI);
	for (int i = 0;uri[i]; i++) if (uri[i] == '/') uri[i] = '_';	

	// prendo gli headers
	content_length = 0;
	for (i = 1; i < j; i++) {
		//printf("%s ===> %s\n", h[i].n, h[i].v);
		if (strcmp("Content-Length", h[i].n) == 0)
			content_length = atoi(h[i].v);  //Atoi:string to integer
		else if (strcmp("Transfer-Encoding", h[i].n) == 0 && strcmp(" chunked", h[i].v) == 0)
			chunked = 1;
		else if (strcmp("Last-Modified",h[i].n) == 0){
			last_modified = h[i].v+1;
		}
	}

	printf("Last-Modified value: \"%s\"\n",last_modified);
	printf("uri: \"%s\"\n", uri);

	if (!expired(uri, last_modified)){
		printf("USING CACHED VERSION\n!");
		printFile(uri);
		return 0;
	}

	//Se la dimensione non è nulla:
	if (content_length != 0) {
		printf("USING WEB VERSION. NOW CACHING!\n");
		//Legge (content_lenghtsocket-i)byte dal socket s partendo dall'indirizzo (entity+i)
		for (i = 0; (i < content_length) && (t = read(s, body + i, content_length - i)); i = i + t)
			;
		//Visualizzo a schermo il BODY
		body[i] = 0;
		printf("%s",body);
		saveToCache(uri,body);

	} else if (chunked) {
		printf("CHUNKED NON GESTITO PER PIGRIZIA\n");
		chunksize = response + i;
		for (k = 0; (t = read(s, response + i + k, 1)) > 0; k++) {
			if (response[i + k] == '\n' && response[i + k - 1] == '\r') {
				response[i + k - 1] = 0;
				dim = hexToLong(chunksize);
				printf("<%s> %ld\n", chunksize, dim);
				for (j = 0; (t = read(s, response + i + k, dim - j)) > 0 && j < dim; j += t, k += t)
					;
				t = read(s, response + i + k, 2);
				//response[i+k+t]=0;
				//printf("***%s***\n",response+i+k);

				k += 2;
				if (dim == 0) {
					printf("Fine body");
					break;
				}
				chunksize = response + i + k;
				k--;
			}
		}
	}
	if (t < 0) {
		perror("read fallita");
		return 1;
	}

	//Visualizzo a schermo il BODY
	//entity[i]=0;
	//printf("%s",entity);
	//free(entity);

	/*
		p = response; 
		while (t=read(s,p,100000)){
		p = p + t;
	 *p=0;
	 }       
	 */
}  //Fine main

long hexToLong(char *str) {
	long val = 0;
	int n, k;
	for (k = 0; str[k]; k++) {
		if (str[k] <= '9' && str[k] >= '0') n = str[k] - '0';
		if (str[k] <= 'Z' && str[k] >= 'A') n = str[k] - 'A' + 10;
		if (str[k] <= 'z' && str[k] >= 'a') n = str[k] - 'a' + 10;
		val = val * 16 + n;
	}
	return val;
}

unsigned short myhtons(unsigned short s) {
	unsigned short tmp = 1;
	unsigned char *p;
	p = (unsigned char *)&tmp;
	if (p[0]) {
		p[0] = ((unsigned char *)&s)[1];
		p[1] = ((unsigned char *)&s)[0];
	} else
		tmp = s;

	return tmp;
}

unsigned char *myhtonx(unsigned char *s, unsigned char *o, int size) {
	unsigned short tmp = 1;
	int i;
	unsigned char *p;
	unsigned char appoggio;
	p = (unsigned char *)&tmp;
	if (p[0])
		for (i = 0; i < size / 2; i++) {
			appoggio = s[i];
			o[i] = s[size - i - 1];
			o[size - i - 1] = appoggio;
		}
	return o;
}

