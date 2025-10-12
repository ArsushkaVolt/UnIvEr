using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Pract2
{
    public struct Technique
    {
        public string Brand { get; set; }    // Свойство для хранения марки техники (например, "Samsung", "Sony")
        public string Type { get; set; }     // Свойство для хранения типа техники (например, "Телевизор", "Холодильник")
        public decimal Price { get; set; }   // Свойство для хранения цены техники в денежных единицах (например, 15000.50)
        public int Rating { get; set; }      // Свойство для хранения рейтинга техники (от 0 до 10 или другой шкалы)
        public int Year { get; set; }        // Свойство для хранения года выпуска техники (например, 2023)
    }

    class Program
    {
        static void Main(string[] args)
        {
            Console.Write("Введите имя файла: ");  // Запрашиваем у пользователя имя файла для работы (например, "data.txt")
            string fileName = Console.ReadLine().Trim();  // Считываем введенное имя файла и убираем лишние пробелы

            LinkedList<Technique> techniques = new LinkedList<Technique>();  // Инициализируем двусвязный список для хранения объектов типа Technique

            if (File.Exists(fileName))  // Проверяем, существует ли файл с указанным именем
            {
                using (StreamReader sr = new StreamReader(fileName))  // Открываем файл для чтения с использованием using для автоматического закрытия
                {
                    string line;
                    while ((line = sr.ReadLine()) != null)  // Читаем файл построчно до конца
                    {
                        string[] parts = line.Split(',');  // Разбиваем строку на части по запятой
                        if (parts.Length == 5)  // Проверяем, что строка содержит ровно 5 полей
                        {
                            Technique t = new Technique
                            {
                                Brand = parts[0].Trim(),           // Извлекаем и обрезаем марку
                                Type = parts[1].Trim(),            // Извлекаем и обрезаем тип
                                Price = decimal.Parse(parts[2].Trim()),  // Преобразуем строку цены в decimal
                                Rating = int.Parse(parts[3].Trim()),     // Преобразуем строку рейтинга в int
                                Year = int.Parse(parts[4].Trim())        // Преобразуем строку года в int
                            };
                            techniques.AddLast(t);  // Добавляем прочитанный объект в конец двусвязного списка
                        }
                    }
                }
            }
            else
            {
                File.Create(fileName).Close();  // Создаем новый пустой файл, если он не существует, и сразу закрываем
            }

            bool exit = false;  // Флаг для управления циклом работы программы
            while (!exit)  // Основной цикл программы
            {
                Console.WriteLine("\nМеню:");  // Выводим заголовок меню
                Console.WriteLine("1. Отобразить содержимое коллекции");
                Console.WriteLine("2. Добавить новый элемент");
                Console.WriteLine("3. Удалить элемент с указанным индексом");
                Console.WriteLine("4. Корректировать элемент");
                Console.WriteLine("5. Работа с коллекцией");
                Console.WriteLine("6. Вывести марки указанного типа не дороже указанной цены с наибольшим рейтингом");
                Console.WriteLine("7. Вывести марки указанного типа, выпущенные в этом году");
                Console.WriteLine("0. Выход");

                string choice = Console.ReadLine().Trim();  // Считываем выбор пользователя
                switch (choice)
                {
                    case "1":
                        DisplayCollection(techniques);  // Вызываем метод для отображения содержимого коллекции в виде таблицы
                        break;
                    case "2":
                        Technique newTechnique = InputTechnique();  // Вызываем метод для ввода нового элемента
                        techniques.AddLast(newTechnique);  // Добавляем новый элемент в конец списка
                        break;
                    case "3":
                        Console.Write("Введите индекс для удаления: ");  // Запрашиваем индекс для удаления
                        if (int.TryParse(Console.ReadLine().Trim(), out int removeIndex) && removeIndex >= 0 && removeIndex < techniques.Count)
                        {
                            RemoveAt(techniques, removeIndex);  // Удаляем элемент по указанному индексу
                        }
                        else
                        {
                            Console.WriteLine("Неверный индекс.");  // Сообщаем об ошибке при неверном индексе
                        }
                        break;
                    case "4":
                        Console.Write("Введите индекс для корректировки: ");  // Запрашиваем индекс для редактирования
                        if (int.TryParse(Console.ReadLine().Trim(), out int editIndex) && editIndex >= 0 && editIndex < techniques.Count)
                        {
                            EditAt(techniques, editIndex);  // Вызываем метод для корректировки элемента
                        }
                        else
                        {
                            Console.WriteLine("Неверный индекс.");  // Сообщаем об ошибке при неверном индексе
                        }
                        break;
                    case "5":
                        CollectionSubMenu(techniques);  // Переходим в подменю для работы с коллекцией
                        break;
                    case "6":
                        Console.Write("Введите тип: ");  // Запрашиваем тип техники для фильтрации
                        string filterType = Console.ReadLine().Trim();
                        Console.Write("Введите максимальную цену: ");  // Запрашиваем максимальную цену
                        if (decimal.TryParse(Console.ReadLine().Trim(), out decimal maxPrice))
                        {
                            var filtered = techniques.Where(t => t.Type.Equals(filterType, StringComparison.OrdinalIgnoreCase) && t.Price <= maxPrice);
                            if (filtered.Any())  // Проверяем, есть ли элементы после фильтрации
                            {
                                int maxRating = filtered.Max(t => t.Rating);  // Находим максимальный рейтинг
                                var maxRatedBrands = filtered.Where(t => t.Rating == maxRating).Select(t => t.Brand).Distinct();
                                Console.WriteLine("Марки с наибольшим рейтингом:");
                                foreach (var brand in maxRatedBrands)  // Выводим уникальные марки с максимальным рейтингом
                                {
                                    Console.WriteLine(brand);
                                }
                            }
                            else
                            {
                                Console.WriteLine("Нет подходящих элементов.");  // Сообщаем, если нет подходящих элементов
                            }
                        }
                        else
                        {
                            Console.WriteLine("Неверный формат цены.");  // Сообщаем об ошибке при неверном формате цены
                        }
                        break;
                    case "7":
                        int currentYear = DateTime.Now.Year;  // Получаем текущий год
                        Console.Write("Введите тип: ");  // Запрашиваем тип техники
                        string yearType = Console.ReadLine().Trim();
                        var thisYearBrands = techniques.Where(t => t.Type.Equals(yearType, StringComparison.OrdinalIgnoreCase) && t.Year == currentYear).Select(t => t.Brand).Distinct();
                        if (thisYearBrands.Any())  // Проверяем, есть ли элементы, выпущенные в текущем году
                        {
                            Console.WriteLine($"Марки, выпущенные в {currentYear}:");
                            foreach (var brand in thisYearBrands)  // Выводим уникальные марки
                            {
                                Console.WriteLine(brand);
                            }
                        }
                        else
                        {
                            Console.WriteLine("Нет подходящих элементов.");  // Сообщаем, если нет подходящих элементов
                        }
                        break;
                    case "0":
                        exit = true;  // Устанавливаем флаг выхода из программы
                        break;
                    default:
                        Console.WriteLine("Неверный выбор.");  // Сообщаем об ошибке при неверном пункте меню
                        break;
                }
            }

            // Сохранение всех данных обратно в файл
            using (StreamWriter sw = new StreamWriter(fileName))  // Открываем файл для записи
            {
                foreach (var t in techniques)  // Проходим по всем элементам списка
                {
                    sw.WriteLine($"{t.Brand},{t.Type},{t.Price},{t.Rating},{t.Year}");  // Записываем данные в формате CSV
                }
            }
        }

        static Technique InputTechnique()
        {
            Technique t = new Technique();  // Создаем новый объект структуры Technique
            Console.Write("Введите марку: ");  // Запрашиваем данные для заполнения
            t.Brand = Console.ReadLine().Trim();
            Console.Write("Введите тип: ");
            t.Type = Console.ReadLine().Trim();
            Console.Write("Введите цену: ");
            decimal.TryParse(Console.ReadLine().Trim(), out decimal price);
            t.Price = price;
            Console.Write("Введите рейтинг: ");
            int.TryParse(Console.ReadLine().Trim(), out int rating);
            t.Rating = rating;
            Console.Write("Введите год выпуска: ");
            int.TryParse(Console.ReadLine().Trim(), out int year);
            t.Year = year;
            return t;  // Возвращаем заполненный объект
        }

        static void DisplayCollection(LinkedList<Technique> list)
        {
            if (list.Count == 0)  // Проверяем, пуста ли коллекция
            {
                Console.WriteLine("Коллекция пуста.");
                return;
            }

            Console.WriteLine("{0,-15} {1,-15} {2,-10} {3,-8} {4,-8}", "Марка", "Тип", "Цена", "Рейтинг", "Год");  // Заголовок таблицы
            int index = 0;
            foreach (var t in list)  // Проходим по всем элементам для вывода
            {
                Console.WriteLine("{0,-15} {1,-15} {2,-10} {3,-8} {4,-8}", t.Brand, t.Type, t.Price, t.Rating, t.Year);
                index++;
            }
        }

        static void RemoveAt(LinkedList<Technique> list, int index)
        {
            LinkedListNode<Technique> node = list.First;  // Начинаем с первого узла
            for (int i = 0; i < index; i++)  // Двигаемся к нужному индексу
            {
                node = node.Next;
            }
            list.Remove(node);  // Удаляем узел
        }

        static void EditAt(LinkedList<Technique> list, int index)
        {
            LinkedListNode<Technique> node = list.First;  // Начинаем с первого узла
            for (int i = 0; i < index; i++)  // Двигаемся к нужному индексу
            {
                node = node.Next;
            }
            node.Value = InputTechnique();  // Заменяем значение узла новым введенным
        }

        static void CollectionSubMenu(LinkedList<Technique> list)
        {
            bool back = false;  // Флаг для выхода из подменю
            while (!back)
            {
                Console.WriteLine("\nРабота с коллекцией:");
                Console.WriteLine("1. Добавить в начало");
                Console.WriteLine("2. Добавить в конец");
                Console.WriteLine("3. Добавить перед указанным индексом");
                Console.WriteLine("4. Добавить после указанного индекса");
                Console.WriteLine("5. Удалить из произвольного места (по индексу)");
                Console.WriteLine("6. Сортировка (по году выпуска)");
                Console.WriteLine("0. Назад");

                string subChoice = Console.ReadLine().Trim();  // Считываем выбор пользователя
                switch (subChoice)
                {
                    case "1":
                        Technique t1 = InputTechnique();  // Ввод нового элемента
                        list.AddFirst(t1);  // Добавление в начало списка
                        break;
                    case "2":
                        Technique t2 = InputTechnique();  // Ввод нового элемента
                        list.AddLast(t2);  // Добавление в конец списка
                        break;
                    case "3":
                        Console.Write("Введите индекс (перед которым добавить): ");  // Запрашиваем индекс
                        if (int.TryParse(Console.ReadLine().Trim(), out int beforeIndex) && beforeIndex >= 0 && beforeIndex <= list.Count)
                        {
                            Technique t3 = InputTechnique();
                            if (beforeIndex == 0)
                            {
                                list.AddFirst(t3);  // Добавление в начало, если индекс 0
                            }
                            else
                            {
                                LinkedListNode<Technique> node = list.First;
                                for (int i = 0; i < beforeIndex; i++)
                                {
                                    node = node.Next;
                                }
                                list.AddBefore(node, t3);  // Добавление перед указанным узлом
                            }
                        }
                        else
                        {
                            Console.WriteLine("Неверный индекс.");  // Сообщаем об ошибке
                        }
                        break;
                    case "4":
                        Console.Write("Введите индекс (после которого добавить): ");  // Запрашиваем индекс
                        if (int.TryParse(Console.ReadLine().Trim(), out int afterIndex) && afterIndex >= 0 && afterIndex < list.Count)
                        {
                            Technique t4 = InputTechnique();
                            LinkedListNode<Technique> node = list.First;
                            for (int i = 0; i < afterIndex; i++)
                            {
                                node = node.Next;
                            }
                            list.AddAfter(node, t4);  // Добавление после указанного узла
                        }
                        else
                        {
                            Console.WriteLine("Неверный индекс.");  // Сообщаем об ошибке
                        }
                        break;
                    case "5":
                        Console.Write("Введите индекс для удаления: ");  // Запрашиваем индекс для удаления
                        if (int.TryParse(Console.ReadLine().Trim(), out int subRemoveIndex) && subRemoveIndex >= 0 && subRemoveIndex < list.Count)
                        {
                            RemoveAt(list, subRemoveIndex);  // Удаляем элемент
                        }
                        else
                        {
                            Console.WriteLine("Неверный индекс.");  // Сообщаем об ошибке
                        }
                        break;
                    case "6":
                        var sortedList = list.OrderBy(t => t.Year).ToList();  // Сортируем список по году выпуска
                        list.Clear();  // Очищаем текущий список
                        foreach (var item in sortedList)  // Заполняем отсортированными элементами
                        {
                            list.AddLast(item);
                        }
                        Console.WriteLine("Коллекция отсортирована по году выпуска.");
                        break;
                    case "0":
                        back = true;  // Возврат в главное меню
                        break;
                    default:
                        Console.WriteLine("Неверный выбор.");  // Сообщаем об ошибке
                        break;
                }
            }
        }
    }
}