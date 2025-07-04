#ifndef STRINGS_RU_H
#define STRINGS_RU_H

#include "resource.h"

static inline void loadStrings_ru() {
    lc_str.app_name = APP_NAME;
    lc_str.app_version = L"Версия " APP_VERSION;
    lc_str.app_dev_name = L"от " APP_DEV_NAME;
    lc_str.application = L"Приложение";
    lc_str.shortcut = L"Ярлык";
    lc_str.file = L"Файл";
    lc_str.folder = L"Папка";
    lc_str.local_drive = L"Локальный диск";
    lc_str.cd_drive = L"CD привод";
    lc_str.computer = L"Компьютер";
    lc_str.desktop = L"Рабочий стол";
    lc_str.documents = L"Документы";
    lc_str.exit = L"Выход";
    lc_str.edit = L"Редактировать";
    lc_str.cut = L"Вырезать";
    lc_str.copy = L"Копировать";
    lc_str.paste = L"Вставить";
    lc_str.paste_shortcut = L"Вставить Ярлык";
    lc_str.select_all = L"Выбрать все";
    lc_str.view = L"Вид";
    lc_str.large_icons = L"Большие Иконки";
    lc_str.small_icons = L"Маленькие Иконки";
    lc_str.list = L"Список";
    lc_str.details = L"Подробности";
    lc_str.help = L"Помощь";
    lc_str.about = L"О программе";
    lc_str.ok = L"OK";
    lc_str.cancel = L"Отмена";
    lc_str.loading = L"Загрузка...";
    lc_str.open = L"Открыть";
    lc_str.create_shortcut = L"Создать Ярлык";
    lc_str.delete = L"Удалить";
    lc_str.rename = L"Переименовать";
    lc_str.new_folder = L"Новая папка";
    lc_str.new_file = L"Новый файл";
    lc_str.items = L"Предметы";
    lc_str.load_iso_image = L"Загрузить ISO-образ";
    lc_str.unload_iso_image = L"Выгрузить ISO-образ";
    lc_str.no_media = L"Нет СМИ";
    lc_str.alert = L"Тревога";
    lc_str.enter_folder_name = L"Введите имя папки:";
    lc_str.enter_file_name = L"Введите имя файла:";
    lc_str.enter_new_name = L"Введите новое имя:";
    lc_str.name = L"Имя";
    lc_str.type = L"Тип";
    lc_str.size = L"Размер";
    lc_str.date = L"Дата";
    lc_str.path = L"Путь";
    lc_str.deleting_files = L"Удаление файлов";
    lc_str.copying_files = L"Копирование файлов";
    lc_str.moving_files = L"Перемещение файлов";
    lc_str.extracting_files = L"Извлечение файлов";
    lc_str.confirm_delete = L"Подтвердить удаление";
    lc_str.confirm_exit = L"Подтвердить выход";
    lc_str.search = L"Поиск";
    lc_str.up = L"Вверх";

    lc_str.fmt_file = L"%ls Файл";
    
    lc_str.msg_invalid_iso_image_file = L"Неверный файл образа ISO!";
    lc_str.msg_deleting_files = L"Удаление файлов, пожалуйста, подождите...";
    lc_str.msg_copying_files = L"Копирование файлов, пожалуйста, подождите...";
    lc_str.msg_moving_files = L"Перемещение файлов, пожалуйста, подождите...";
    lc_str.msg_extracting_files = L"Извлечение файлов, пожалуйста, подождите...";
    lc_str.msg_cancel_file_operation = L"Вы хотите отменить операцию?";
    lc_str.msg_confirm_delete_item = L"Вы уверены, что хотите удалить \"%ls\"?";
    lc_str.msg_confirm_delete_multiple_items = L"Вы уверены, что хотите удалить %d элементов?";
    lc_str.msg_confirm_exit_app = L"Вы уверены, что хотите выйти?";
}

#endif