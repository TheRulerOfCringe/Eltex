В UDP нет подключения, как в TCP, так что получили сообщение от клиента - зарегистрировали, как user1
Второй что-то прислал - зарегистрировали, как user2
(УЖЕ НЕТ ТАКИХ ПРОБЛЕМ, Я ПОФИКСИЛ)Есть некоторые проблемки с выходом по сигналу, server завершается только после ещё одного сообщения
В client используется select(), причём sockfd + 1, потому что он проверяет от 0 до ведённого не включительно
Сервер запускается по команде ./server
Клиент запускается по команде ./client <IP>

Дополнение:
IP сервера можно узнать командой
hostname -I
Или, если тестишь клиента и сервер на одной машине, то можешь использовать
127.0.0.1

                             ------ Wireshark ------
                             
Открываем через sudo в терминале
В главном меню жамкаем "any", а то опять запаникуешь, что ничего не capture'ит
Всё готово

                                ------ UDP ------
                                
Важная структура адреса сокета
struct sockaddr_in {
    sa_family_t    sin_family;   // Семейство адресов (AF_INET для IPv4)
    in_port_t      sin_port;     // Порт в сетевом порядке байт
    struct in_addr sin_addr;     // IP-адрес
    unsigned char  sin_zero[8];  // Дополнение до размера sockaddr
};

                       ------ Преобразование портов ------

=== 16-bit порты (htons/ntohs) ===
Десятичный вид: 12345
Шестнадцатеричный: 0x3039
Host order           : 3039 = 39 30 
Network order (htons): 3039 = 30 39 
Back to host (ntohs) : 3039 = 39 30 

=== 32-bit IP адреса (htonl/ntohl) ===
IP адрес: 192.168.0.1
Шестнадцатеричный: 0xC0A80001
Host order           : C0A80001 = 01 00 A8 C0 
Network order (htonl): C0A80001 = C0 A8 00 01 
Back to host (ntohl) : C0A80001 = 01 00 A8 C0 

                                ------ Сервер ------

1) Создаём UDP сокет (socket)

// socket(domain, type, protocol)
// PF_INET / AF_INET - IPv4 протокол
// SOCK_DGRAM - датаграммный сокет (UDP)
// 0 - протокол по умолчанию для SOCK_DGRAM (UDP)
sockfd = socket(PF_INET, SOCK_DGRAM, 0);

НОВИНКА
1.5) Ставим флаг таймаута

struct timeval tv;
tv.tv_sec = 1;    // 1 секунда
tv.tv_usec = 0;   // 0 микросекунд
setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

2) Привязываем сокет (bind)

// Заполнение структуры адреса сервера
bzero(&servaddr, sizeof(servaddr));  // Обнуление структуры
servaddr.sin_family = AF_INET;       // IPv4
servaddr.sin_port = htons(PORT);     // Порт 12345 в сетевом порядке
servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  // Принимаем со всех интерфейсов

// Привязка сокета к адресу
bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

3) Принимаем пакеты (recvfrom)

n = recvfrom(sockfd, buffer, BUFFER_SIZE-1, 0, (struct sockaddr *) &cliaddr, &clilen)

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);

// Параметры:
// sockfd - дескриптор сокета
// buf - буфер для приема данных
// len - размер буфера
// flags - флаги (0 - по умолчанию)
// src_addr - структура для адреса отправителя
// addrlen - указатель на размер структуры адреса

// Возвращает:
// >0 - количество принятых байт
// -1 - ошибка

4) Регистрируем клиентов по первому сообщению от них

5) Работаем, пока:
  5.1) Сервер не нажмёт Ctrl+C, отправив клиентам уведомление, чтобы тоже закрылись
  5.2) Пока один клиент не напишет exit

6) Завершаем работу

close(sockfd);



















