#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#include <pthread.h>

#define MSG struct mMessage
#define SMSG struct mSystemMessage
/*
        SERVER_QKEY - ключ очереди сервисных сообщений
        MESSAGE_QKEY - ключ очереди сообщений
*/
#define SERVER_QKEY 2658
#define MESSAGE_QKEY 2659

/*
        d_type - кто должен принять
                SERVER_T - адрес сервера, клиенты получат id. Обязательно 0 < id < SERVER_T
                при регистрации id неизвестного равен SERVER_T + 1. Перед принятием сообщения, клиент сравнивает свое имя с тем, что в сообщении
        msg_fl - тип сообщения
        data_int - поле для числа
        data_str - поле для строки
        data_str_long - поля для длинной строки
        MSG_SIZE - размер MSG
        SMSG_SIZE - размер SMSG
*/
#define SERVER_T 15
#define LONG_STR_SIZE 256
#define STR_SIZE 32
#define MSG_SIZE sizeof(struct mMessage)
#define SMSG_SIZE sizeof(struct mSystemMessage)
MSG {
        long d_type;
        int data_int;
        char data_str_long[LONG_STR_SIZE];
        char msg_fl;
};

SMSG {
        long d_type;
        int data_int;
        char data_str[STR_SIZE];
        char msg_fl;
};

/*
        Типы сервисных сообщений
        (c)RREG - запрос на регистрацию
                SERVER_T
                -
                имя клиента
        (s)SREG - успешная регистрация
                SERVER_T + 1
                новый id клиента
                имя клиента
        (s)EREG - ошибка регистрации
                SERVER_T + 1
                номер ошибки (ERNAME - имя уже существует, ERCL - нет места для нового клента)
                имя клиента
        (c)CEXIT - клиент вышел
                SERVER_T
                id клиента
                -
        (s)CAWAY - клиента выкинули
                id клиента
                -
                -
        (s)CREQ - проверка клиента
                id клиента
                -
                -
        (c)CREQE - ответ на проверу
                SERVER_T
                id клиента
                -
*/
#define RREG 1
#define SREG 2
#define EREG 3
#define CEXIT 4
#define CAWAY 5
#define CREQ 6
#define CREQE 7

#define ERNAME 1
#define ERCL 2
/*
        Типы сообщений чата
        (s)CNEW - новый клиент
                id
                id новичка
                имя клиента
        (c)MNEW - новое сообщение от клиента
                SERVER_T
                id от кого
                сообщение
        (s)MNEWB - новое сообщение (рассылка всем)
                id
                id от кого
                сообщение
        (s)COUT - клиент вышел из чата
                id
                id того, кто вышел
                -
*/
#define CNEW 1
#define MNEW 2
#define MNEWB 3
#define COUT 4