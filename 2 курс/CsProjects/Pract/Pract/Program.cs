using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pract
{
    // Интерфейс для задания 3, определяет методы для прослушивания звука и получения имени класса
    public interface IListenable
    {
        void Listen();
        string GetClassName();
    }

    // Абстрактный базовый класс для иерархии устройств (задание 2)
    public abstract class Device : IListenable
    {
        // Приватное поле для производителя
        private string manufacturer;
        // Свойство для доступа к производителю
        public string Manufacturer
        {
            get => manufacturer;
            set => manufacturer = value;
        }

        // Абстрактный метод использования устройства, должен быть реализован в дочерних классах
        public abstract void Use();
        // Виртуальный метод для получения имени класса, может быть переопределён
        public virtual string GetClassName()
        {
            return "Device";
        }

        // Реализация метода интерфейса для прослушивания звука устройства
        public void Listen()
        {
            Console.WriteLine($"{GetClassName()} издает звук работы.");
        }
    }

    // Класс Телевизор для задания 1, наследуется от Device
    public class Television : Device
    {
        // Приватные поля для свойств телевизора
        private string brand;
        private double diagonal;
        private decimal price;
        // Поле только для чтения для серийного номера
        private readonly string serialNumber;
        // Константа для максимальной диагонали
        private const double MAX_DIAGONAL = 100.0;

        // Свойство для марки телевизора
        public string Brand
        {
            get => brand;
            set => brand = value;
        }

        // Свойство для диагонали с проверкой допустимых значений
        public double Diagonal
        {
            get => diagonal;
            set => diagonal = value > 0 && value <= MAX_DIAGONAL ? value : throw new ArgumentException("Недопустимая диагональ");
        }

        // Свойство для цены с проверкой неотрицательности
        public decimal Price
        {
            get => price;
            set => price = value >= 0 ? value : throw new ArgumentException("Цена не может быть отрицательной");
        }

        // Свойство только для чтения для серийного номера
        public string SerialNumber => serialNumber;

        // Конструктор по умолчанию
        public Television()
        {
            brand = "Unknown";
            diagonal = 32.0;
            price = 0;
            serialNumber = Guid.NewGuid().ToString();
            Manufacturer = "Unknown";
        }

        // Конструктор с параметрами
        public Television(string brand, double diagonal, decimal price, string manufacturer)
        {
            this.brand = brand;
            this.diagonal = diagonal;
            this.price = price;
            this.serialNumber = Guid.NewGuid().ToString();
            Manufacturer = manufacturer;
        }

        // Конструктор с частичными параметрами
        public Television(string brand, string manufacturer)
        {
            this.brand = brand;
            this.diagonal = 32.0;
            this.price = 0;
            this.serialNumber = Guid.NewGuid().ToString();
            Manufacturer = manufacturer;
        }

        // Метод для просмотра телевизора
        public void Watch()
        {
            Console.WriteLine($"Смотрим телевизор {Brand} с диагональю {Diagonal} дюймов.");
        }

        // Метод для включения телевизора
        public void TurnOn()
        {
            Console.WriteLine($"Телевизор {Brand} включен.");
        }

        // Метод для выключения телевизора
        public void TurnOff()
        {
            Console.WriteLine($"Телевизор {Brand} выключен.");
        }

        // Статический метод для прослушивания звука работы телевизора
        public static void ListenOperation()
        {
            Console.WriteLine("Телевизор издает звук работы: *шум вентилятора*");
        }

        // Перегрузка метода ToString для вывода информации о телевизоре
        public override string ToString()
        {
            return $"Телевизор: Марка={Brand}, Диагональ={Diagonal}\", Цена={Price}, Производитель={Manufacturer}, Серийный номер={SerialNumber}";
        }

        // Перегрузка операции сравнения ==
        public static bool operator ==(Television tv1, Television tv2)
        {
            if (ReferenceEquals(tv1, null) || ReferenceEquals(tv2, null))
                return ReferenceEquals(tv1, tv2);
            return tv1.Price == tv2.Price && tv1.Diagonal == tv2.Diagonal;
        }

        // Перегрузка операции сравнения !=
        public static bool operator !=(Television tv1, Television tv2)
        {
            return !(tv1 == tv2);
        }

        // Перегрузка арифметической операции + для увеличения цены
        public static Television operator +(Television tv, decimal priceIncrease)
        {
            return new Television(tv.Brand, tv.Diagonal, tv.Price + priceIncrease, tv.Manufacturer);
        }

        // Переопределение Equals для сравнения объектов
        public override bool Equals(object obj)
        {
            if (obj is Television tv)
                return this == tv;
            return false;
        }

        // Переопределение GetHashCode для корректной работы Equals
        public override int GetHashCode()
        {
            return HashCode.Combine(Price, Diagonal);
        }

        // Переопределение метода GetClassName с вызовом базового
        public override string GetClassName()
        {
            return $"Television ({base.GetClassName()})";
        }

        // Переопределение метода Use
        public override void Use()
        {
            Console.WriteLine($"Используем телевизор {Brand} для просмотра.");
        }
    }

    // Класс Телефон, наследуется от Device
    public class Phone : Device
    {
        // Свойство для модели телефона
        public string Model { get; set; }

        // Конструктор
        public Phone(string model, string manufacturer)
        {
            Model = model;
            Manufacturer = manufacturer;
        }

        // Метод для совершения звонка
        public void Call()
        {
            Console.WriteLine($"Звоним с телефона {Model}.");
        }

        // Переопределение метода GetClassName с вызовом базового
        public override string GetClassName()
        {
            return $"Phone ({base.GetClassName()})";
        }

        // Переопределение метода Use
        public override void Use()
        {
            Console.WriteLine($"Используем телефон {Model} для звонка.");
        }
    }

    // Класс Наушники, наследуется от Device
    public class Headphones : Device
    {
        // Свойство, указывающее, беспроводные ли наушники
        public bool IsWireless { get; set; }

        // Конструктор
        public Headphones(bool isWireless, string manufacturer)
        {
            IsWireless = isWireless;
            Manufacturer = manufacturer;
        }

        // Метод для воспроизведения музыки
        public void PlayMusic()
        {
            Console.WriteLine($"Воспроизводим музыку через {(IsWireless ? "беспроводные" : "проводные")} наушники.");
        }

        // Переопределение метода GetClassName с вызовом базового
        public override string GetClassName()
        {
            return $"Headphones ({base.GetClassName()})";
        }

        // Переопределение метода Use
        public override void Use()
        {
            Console.WriteLine($"Используем наушники для прослушивания музыки.");
        }
    }

    // Класс Патефон, sealed, наследуется от Device
    public sealed class Gramophone : Device
    {
        // Свойство для скорости вращения пластинки
        public int RecordSpeed { get; set; }

        // Конструктор
        public Gramophone(int recordSpeed, string manufacturer)
        {
            RecordSpeed = recordSpeed;
            Manufacturer = manufacturer;
        }

        // Метод для проигрывания пластинки
        public void PlayRecord()
        {
            Console.WriteLine($"Патефон играет пластинку на скорости {RecordSpeed} об/мин.");
        }

        // Переопределение метода GetClassName с вызовом базового
        public override string GetClassName()
        {
            return $"Gramophone ({base.GetClassName()})";
        }

        // Переопределение метода Use
        public override void Use()
        {
            Console.WriteLine($"Используем патефон для проигрывания пластинки.");
        }
    }

    // Класс Собака для задания 3, реализует интерфейс IListenable
    public class Dog : IListenable
    {
        // Свойство для имени собаки
        public string Name { get; set; }

        // Конструктор
        public Dog(string name)
        {
            Name = name;
        }

        // Реализация метода Listen из интерфейса
        public void Listen()
        {
            Console.WriteLine($"Собака {Name} издает звук: Гав-гав!");
        }

        // Реализация метода GetClassName из интерфейса
        public string GetClassName()
        {
            return "Dog";
        }
    }

    // Основной класс программы
    class Program
    {
        // Главный метод программы
        static void Main(string[] args)
        {
            // Основной цикл меню
            while (true)
            {
                Console.WriteLine("\nВыберите задание:");
                Console.WriteLine("1. Тестирование класса Телевизор");
                Console.WriteLine("2. Тестирование иерархии классов");
                Console.WriteLine("3. Тестирование интерфейса и списка");
                Console.WriteLine("0. Выход");
                Console.Write("Ваш выбор: ");
                string choice = Console.ReadLine();

                if (choice == "0") break;

                // Выбор задания
                switch (choice)
                {
                    case "1":
                        TestTelevision();
                        break;
                    case "2":
                        TestHierarchy();
                        break;
                    case "3":
                        TestInterface();
                        break;
                    default:
                        Console.WriteLine("Неверный выбор. Попробуйте снова.");
                        break;
                }
            }
        }

        // Метод для тестирования класса Телевизор (задание 1)
        static void TestTelevision()
        {
            Television tv = new Television();
            while (true)
            {
                Console.WriteLine("\nМеню тестирования класса Телевизор:");
                Console.WriteLine("1. Задать параметры телевизора");
                Console.WriteLine("2. Вывести свойства телевизора");
                Console.WriteLine("3. Выполнить статический метод");
                Console.WriteLine("4. Выполнить методы телевизора");
                Console.WriteLine("0. Назад");
                Console.Write("Ваш выбор: ");
                string choice = Console.ReadLine();

                if (choice == "0") break;

                switch (choice)
                {
                    case "1":
                        // Задание параметров телевизора
                        Console.Write("Введите марку: ");
                        string brand = Console.ReadLine();
                        Console.Write("Введите диагональ (дюймы): ");
                        double diagonal = double.Parse(Console.ReadLine());
                        Console.Write("Введите цену: ");
                        decimal price = decimal.Parse(Console.ReadLine());
                        Console.Write("Введите производителя: ");
                        string manufacturer = Console.ReadLine();
                        tv = new Television(brand, diagonal, price, manufacturer);
                        break;
                    case "2":
                        // Вывод свойств телевизора
                        Console.WriteLine(tv.ToString());
                        break;
                    case "3":
                        // Выполнение статического метода
                        Television.ListenOperation();
                        break;
                    case "4":
                        // Выполнение методов объекта
                        tv.Watch();
                        tv.TurnOn();
                        tv.TurnOff();
                        break;
                    default:
                        Console.WriteLine("Неверный выбор. Попробуйте снова.");
                        break;
                }
            }
        }

        // Метод для тестирования иерархии классов (задание 2)
        static void TestHierarchy()
        {
            // Создание объектов каждого класса
            Television tv = new Television("Samsung", 55.0, 1000, "Korea");
            Phone phone = new Phone("iPhone", "USA");
            Headphones headphones = new Headphones(true, "Japan");
            Gramophone gramophone = new Gramophone(78, "Germany");

            Device[] devices = { tv, phone, headphones, gramophone };

            while (true)
            {
                Console.WriteLine("\nМеню тестирования иерархии классов:");
                Console.WriteLine("1. Задать свойства объектов");
                Console.WriteLine("2. Вывести свойства объектов");
                Console.WriteLine("3. Выполнить методы объектов");
                Console.WriteLine("4. Вывести имена классов");
                Console.WriteLine("0. Назад");
                Console.Write("Ваш выбор: ");
                string choice = Console.ReadLine();

                if (choice == "0") break;

                switch (choice)
                {
                    case "1":
                        // Задание свойств для каждого объекта
                        Console.Write("Марка телевизора: ");
                        tv.Brand = Console.ReadLine();
                        Console.Write("Диагональ телевизора: ");
                        tv.Diagonal = double.Parse(Console.ReadLine());
                        Console.Write("Цена телевизора: ");
                        tv.Price = decimal.Parse(Console.ReadLine());
                        Console.Write("Производитель телевизора: ");
                        tv.Manufacturer = Console.ReadLine();

                        Console.Write("Модель телефона: ");
                        phone.Model = Console.ReadLine();
                        Console.Write("Производитель телефона: ");
                        phone.Manufacturer = Console.ReadLine();

                        Console.Write("Беспроводные наушники (true/false): ");
                        headphones.IsWireless = bool.Parse(Console.ReadLine());
                        Console.Write("Производитель наушников: ");
                        headphones.Manufacturer = Console.ReadLine();

                        Console.Write("Скорость пластинки патефона: ");
                        gramophone.RecordSpeed = int.Parse(Console.ReadLine());
                        Console.Write("Производитель патефона: ");
                        gramophone.Manufacturer = Console.ReadLine();
                        break;
                    case "2":
                        // Вывод свойств всех объектов
                        Console.WriteLine(tv.ToString());
                        Console.WriteLine($"Телефон: Модель={phone.Model}, Производитель={phone.Manufacturer}");
                        Console.WriteLine($"Наушники: Беспроводные={headphones.IsWireless}, Производитель={headphones.Manufacturer}");
                        Console.WriteLine($"Патефон: Скорость={gramophone.RecordSpeed}, Производитель={gramophone.Manufacturer}");
                        break;
                    case "3":
                        // Выполнение методов всех объектов
                        tv.Use();
                        phone.Use();
                        headphones.Use();
                        gramophone.Use();
                        break;
                    case "4":
                        // Вывод имен классов
                        foreach (var device in devices)
                        {
                            Console.WriteLine(device.GetClassName());
                        }
                        break;
                    default:
                        Console.WriteLine("Неверный выбор. Попробуйте снова.");
                        break;
                }
            }
        }

        // Метод для тестирования интерфейса и списка (задание 3)
        static void TestInterface()
        {
            // Создание списка объектов, реализующих интерфейс IListenable
            List<IListenable> listenables = new List<IListenable>
            {
                new Television("Sony", 42.0, 800, "Japan"),
                new Phone("Nokia", "Finland"),
                new Headphones(false, "China"),
                new Gramophone(33, "USA"),
                new Dog("Rex")
            };

            while (true)
            {
                Console.WriteLine("\nМеню тестирования интерфейса:");
                Console.WriteLine("1. Добавить новый объект");
                Console.WriteLine("2. Вывести свойства объекта");
                Console.WriteLine("3. Выполнить метод Listen объекта");
                Console.WriteLine("4. Вывести все объекты с именами классов");
                Console.WriteLine("5. Выполнить функцию с объектом");
                Console.WriteLine("0. Назад");
                Console.Write("Ваш выбор: ");
                string choice = Console.ReadLine();

                if (choice == "0") break;

                switch (choice)
                {
                    case "1":
                        // Добавление нового объекта в список
                        Console.WriteLine("Выберите тип объекта:");
                        Console.WriteLine("1. Телевизор");
                        Console.WriteLine("2. Телефон");
                        Console.WriteLine("3. Наушники");
                        Console.WriteLine("4. Патефон");
                        Console.WriteLine("5. Собака");
                        Console.Write("Ваш выбор: ");
                        string typeChoice = Console.ReadLine();

                        switch (typeChoice)
                        {
                            case "1":
                                Console.Write("Марка: ");
                                string brand = Console.ReadLine();
                                Console.Write("Диагональ: ");
                                double diagonal = double.Parse(Console.ReadLine());
                                Console.Write("Цена: ");
                                decimal price = decimal.Parse(Console.ReadLine());
                                Console.Write("Производитель: ");
                                string manufacturer = Console.ReadLine();
                                listenables.Add(new Television(brand, diagonal, price, manufacturer));
                                break;
                            case "2":
                                Console.Write("Модель: ");
                                string model = Console.ReadLine();
                                Console.Write("Производитель: ");
                                manufacturer = Console.ReadLine();
                                listenables.Add(new Phone(model, manufacturer));
                                break;
                            case "3":
                                Console.Write("Беспроводные (true/false): ");
                                bool isWireless = bool.Parse(Console.ReadLine());
                                Console.Write("Производитель: ");
                                manufacturer = Console.ReadLine();
                                listenables.Add(new Headphones(isWireless, manufacturer));
                                break;
                            case "4":
                                Console.Write("Скорость пластинки: ");
                                int speed = int.Parse(Console.ReadLine());
                                Console.Write("Производитель: ");
                                manufacturer = Console.ReadLine();
                                listenables.Add(new Gramophone(speed, manufacturer));
                                break;
                            case "5":
                                Console.Write("Имя собаки: ");
                                string name = Console.ReadLine();
                                listenables.Add(new Dog(name));
                                break;
                            default:
                                Console.WriteLine("Неверный выбор.");
                                break;
                        }
                        break;
                    case "2":
                        // Вывод свойств объекта по индексу
                        Console.Write("Введите индекс объекта (0-{0}): ", listenables.Count - 1);
                        int index = int.Parse(Console.ReadLine());
                        if (index >= 0 && index < listenables.Count)
                        {
                            var obj = listenables[index];
                            switch (obj)
                            {
                                case Television tv:
                                    Console.WriteLine(tv.ToString());
                                    break;
                                case Phone phone:
                                    Console.WriteLine($"Телефон: Модель={phone.Model}, Производитель={phone.Manufacturer}");
                                    break;
                                case Headphones headphones:
                                    Console.WriteLine($"Наушники: Беспроводные={headphones.IsWireless}, Производитель={headphones.Manufacturer}");
                                    break;
                                case Gramophone gramophone:
                                    Console.WriteLine($"Патефон: Скорость={gramophone.RecordSpeed}, Производитель={gramophone.Manufacturer}");
                                    break;
                                case Dog dog:
                                    Console.WriteLine($"Собака: Имя={dog.Name}");
                                    break;
                            }
                        }
                        else
                        {
                            Console.WriteLine("Неверный индекс.");
                        }
                        break;
                    case "3":
                        // Выполнение метода Listen для объекта по индексу
                        Console.Write("Введите индекс объекта (0-{0}): ", listenables.Count - 1);
                        index = int.Parse(Console.ReadLine());
                        if (index >= 0 && index < listenables.Count)
                        {
                            listenables[index].Listen();
                        }
                        else
                        {
                            Console.WriteLine("Неверный индекс.");
                        }
                        break;
                    case "4":
                        // Вывод всех объектов с их именами классов
                        for (int i = 0; i < listenables.Count; i++)
                        {
                            Console.WriteLine($"[{i}] {listenables[i].GetClassName()}");
                        }
                        break;
                    case "5":
                        // Выполнение функции с объектом по индексу
                        Console.Write("Введите индекс объекта (0-{0}): ", listenables.Count - 1);
                        index = int.Parse(Console.ReadLine());
                        if (index >= 0 && index < listenables.Count)
                        {
                            ProcessListenable(listenables[index]);
                        }
                        else
                        {
                            Console.WriteLine("Неверный индекс.");
                        }
                        break;
                    default:
                        Console.WriteLine("Неверный выбор. Попробуйте снова.");
                        break;
                }
            }
        }

        // Функция для обработки объекта, реализующего интерфейс IListenable
        static void ProcessListenable(IListenable listenable)
        {
            Console.WriteLine($"Обрабатываем объект класса {listenable.GetClassName()}:");
            listenable.Listen();
        }
    }
}



