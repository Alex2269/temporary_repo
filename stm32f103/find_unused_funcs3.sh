#!/bin/bash

# 1. Збираємо список всіх функцій з файлів .c
grep -hoP '^\w+\s+\w+\s*\(' $(find . -name "*.c") > all_funcs.txt

# 2. Перевіряємо кожну функцію на використання в інших файлах
echo "Невикористані функції:"

while read -r func; do
    # Пропускаємо функції без ім'я або з коментарями
    [[ -z "$func" || "$func" =~ ^// ]] && continue
    # Перевіряємо, чи не використовується функція в інших місцях (окрім визначення)
    count=$(grep -r "$func" $(find . -name "*.c") $(find . -name "*.h") | grep -v "$func *(" | wc -l)
    if [ "$count" -eq 0 ]; then
        echo "$func"
    fi
done < all_funcs.txt
