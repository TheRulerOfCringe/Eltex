1) Проверка, что программа запущенена суперюзером

getuid() != 0

2) Создаём RAW-сокет

raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

Параметры:
AF_INET — IPv4
SOCK_RAW — тип сокета, собсна RAW-сокет
IPPROTO_UDP — фильтр протокола (только UDP пакеты)

3) НИКАКОГО bind()

bind() привязывает сокет к порту.
А у нас RAW-сокет, он все порты читает.

4) Установка таймаута

struct timeval tv;
tv.tv_sec = 1;   // 1 секунда таймаута для корректного выхода по SIGINT
tv.tv_usec = 0;
setsockopt(raw_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

Сперва файловый дескриптор сокета
Потом уровень, у которого меняем параметры:
SOL_SOCKET	Уровень сокета (общие настройки)	Таймауты, буферы, опции сокета
IPPROTO_IP	Уровень IP	                        TTL, multicast, маршрутизация
IPPROTO_TCP	Уровень TCP	                        Nagle, keepalive, congestion
IPPROTO_UDP	Уровень UDP	                        Checksum, multicast

Поэтому выбираем SOL_SOCKET, ставим имя опции SO_RCVTIMEO (receive timeout)
И передаём структурку с временем введённым

5) Получаем пакет

recv(raw_socket, buffer, BUFFER_SIZE, 0)

raw_socket — дескриптор RAW-сокета
buffer — буфер, ясен красен, в него запишем данные
BUFFER_SIZE — Максимальное число байт, которые можем принять
0 — стандартный режим
На лекции вы так и сказали "Лучше ставьте 0"

6) Анализируем

decode_udp_packet(buffer, data_size, dump_file, decoded_file)

Мы не декапсулировали данные, так что там остаются все заголовки.
