[![Join the chat at https://gitter.im/RegEx1CAddin/Lobby](https://badges.gitter.im/RegEx1CAddin.svg)](https://gitter.im/RegEx1CAddin/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

# RegEx1CAddin

Внешняя Native API компонента для выполнения регулярных выражений на платформе 1С:Предприятие 8. Написана на C++. Используется движок из стандартной библиотеки шаблонов C++11 - std::regex.
Внимание! Текущая версия является тестовой и еще не была протестирована на реальных проектах и базах. 

Текущая версия собрана для следующих платформ:
Windows 32bit 
Windows 64bit 
Linux 32bit 
Linux 64bit 

Тестировалось на платформе 8.3.12.1567 (Windows XP SP3, Windows 7, Windows Server 2008 R2, Ubuntu 14 32-64bit)

Сборка осуществлялась с использованием следующих инструментов:

Под Windows: Microsoft Visual Studio Community 2017

Под Linux: GCC 6

Использовалась статическая сборка, поэтому компонента не требует установки каких-либо дополнительных библиотек.

Компонента реализует следующие методы:

### Метод НайтиСовпадения / Matches(<Текст для анализа>, <Регулярное выражение>).

Метод выполняет поиск в переданном тексте по переданному регулярному выражению.

Результатом выполнения метода будет массив результатов поиска. Каждый элемент массива - найденная подгруппа поиска. Если подгрупп нет, то массив будет содержать один элемент - найденную строку.

Возвращаемое значение: Ничего не возвращает.

Для того, чтобы получить результаты выполнения метода (массив результатов), необходимо выполнить метод Следующий/Next(), и после этого, в свойстве ТекущееЗначение/CurrentValue будет доступно значение текущей подгруппы результата выполнения (текущий элемент массива результатов). Идея  похожа на обход результата запроса.

Пример:

Рег.НайтиСовпадения("Hello world", "([A-Za-z]+)\s+([a-z]+)");

Пока Рег.Следующий() Цикл
    Сообщить(Рег.ТекущееЗначение);    
КонецЦикла; 
Результат будет содержать 3 элемента:

Hello world

Hello

world


### Метод Количество()/Count()

Возвращает количество результатов поиска, после выполнения метода НайтиСовпадения / Matches

### Метод Заменить/Replace(<Текст для анализа>, <Регулярное выражение>, <Значение для замены>).

Заменяет в переданном тексте часть, соответствующую регулярному выражению, значением, переданным третьим параметром.

Возвращаемое значение: Строка, результат замены.

### Метод Совпадает/IsMatch(<Текст для анализа>, <Регулярное выражение>).

Делает проверку на соответствие текста регулярному выражению.

Возвращаемое значение: Булево. Возвращает значение Истина если текст соответствует регулярному выражению.

Пример использования:

Предполагается что архив с компонентами был загружен в общий макет "RegEx"

УстановитьВнешнююКомпоненту("ОбщийМакет.RegEx");
ПодключитьВнешнююКомпоненту("ОбщийМакет.RegEx", "Component", ТипВнешнейКомпоненты.Native);
            
Рег = Новый("AddIn.Component.RegEx");
Рег.НайтиСовпадения("Hello world", "([A-Za-z]+)\s+([a-z]+)");

Пока Рег.Следующий() Цикл
    Сообщить(Рег.ТекущееЗначение);    
КонецЦикла; 

Сообщить(Рег.Количество());

Сообщить(Рег.Совпадает("Hello world", "([A-Za-z]+)\s+([a-z]+)"));

Сообщить(Рег.Заменить("Hello world", "([A-Za-z]+)\s+([a-z]+)", "Текст для замены"));
