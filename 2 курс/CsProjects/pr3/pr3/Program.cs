using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace FileLibraryTask
{
    // ====================== КЛАСС-ОБЁРТКА ======================
    public class FileWrapper : IDisposable
    {
        private IntPtr _handle = IntPtr.Zero;
        private bool _disposed = false;
        private readonly string _path;

        // ЖЁСТКО только 64-битная версия библиотеки
        private const string DLL_NAME = "file64.dll";

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr open(string path, bool read);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void close(IntPtr file);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern bool read(IntPtr file, int num, StringBuilder word);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void write(IntPtr file, string text);

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern int length(IntPtr file);

        public FileWrapper(string path, bool readOnly)
        {
            if (string.IsNullOrWhiteSpace(path))
                throw new ArgumentException("Путь к файлу не может быть пустым.", nameof(path));

            _path = path;
            _handle = open(path, readOnly);

            if (_handle == IntPtr.Zero)
                throw new InvalidOperationException($"Не удалось открыть файл: {path}");
        }

        public int WordCount => length(_handle);

        public string GetWord(int index) // индекс с 1
        {
            if (index < 1)
                throw new ArgumentOutOfRangeException(nameof(index), "Нумерация слов начинается с 1");

            var sb = new StringBuilder(255);
            bool success = read(_handle, index, sb);

            if (!success)
                throw new IndexOutOfRangeException($"Слово с номером {index} отсутствует в файле.");

            return sb.ToString().TrimEnd('\0');
        }

        public List<string> ReadAllWords()
        {
            int count = WordCount;
            var words = new List<string>(count);

            for (int i = 1; i <= count; i++)
                words.Add(GetWord(i));

            return words;
        }

        public void RewriteFile(IEnumerable<string> words)
        {
            if (words == null) throw new ArgumentNullException(nameof(words));
            string content = string.Join(" ", words.Where(w => !string.IsNullOrWhiteSpace(w)));
            write(_handle, content);
        }

        public void Dispose()
        {
            if (!_disposed && _handle != IntPtr.Zero)
            {
                close(_handle);
                _handle = IntPtr.Zero;
                _disposed = true;
            }
        }

        ~FileWrapper()
        {
            Dispose();
        }
    }

    // ====================== ОСНОВНАЯ ПРОГРАММА ======================
    class Program
    {
        static FileWrapper file1 = null;
        static FileWrapper file2 = null;

        static void Main(string[] args)
        {
            Console.OutputEncoding = Encoding.UTF8;

            while (true)
            {
                Console.WriteLine("\n╔══════════════════════════════════════════╗");
                Console.WriteLine("║             МЕНЮ ПРОГРАММЫ               ║");
                Console.WriteLine("╚══════════════════════════════════════════╝");
                Console.WriteLine("1. Открыть файл 21.txt");
                Console.WriteLine("2. Открыть файл 22.txt");
                Console.WriteLine("3. Показать количество слов");
                Console.WriteLine("4. Отсортировать слова по длине в обоих файлах");
                Console.WriteLine("5. Перекрестная замена слов");
                Console.WriteLine("6. Выход");
                Console.Write("\nВыберите действие: ");

                string choice = Console.ReadLine()?.Trim();

                try
                {
                    switch (choice)
                    {
                        case "1":
                            file1?.Dispose();
                            file1 = new FileWrapper("21.txt", false);
                            Console.WriteLine("21.txt успешно открыт на чтение/запись.");
                            break;

                        case "2":
                            file2?.Dispose();
                            file2 = new FileWrapper("22.txt", false);
                            Console.WriteLine("22.txt успешно открыт на чтение/запись.");
                            break;

                        case "3":
                            if (file1 != null) Console.WriteLine($"21.txt: {file1.WordCount} слов");
                            if (file2 != null) Console.WriteLine($"22.txt: {file2.WordCount} слов");
                            if (file1 == null && file2 == null) Console.WriteLine("Файлы не открыты.");
                            break;

                        case "4":
                            SortFileByLength("21.txt", ref file1);
                            SortFileByLength("22.txt", ref file2);
                            break;

                        case "5":
                            if (file1 == null || file2 == null)
                            {
                                Console.WriteLine("Ошибка: оба файла должны быть открыты!");
                                break;
                            }
                            CrossReplaceWords();
                            // Переоткрываем файлы после изменений
                            file1?.Dispose();
                            file2?.Dispose();
                            file1 = new FileWrapper("21.txt", false);
                            file2 = new FileWrapper("22.txt", false);
                            Console.WriteLine("Перекрестная замена выполнена.");
                            break;

                        case "6":
                            file1?.Dispose();
                            file2?.Dispose();
                            Console.WriteLine("До свидания!");
                            return;

                        default:
                            Console.WriteLine("Неверный пункт меню.");
                            break;
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"ОШИБКА: {ex.Message}");
                }
            }
        }

        static void SortFileByLength(string path, ref FileWrapper wrapper)
        {
            if (!System.IO.File.Exists(path))
            {
                Console.WriteLine($"Файл {path} не найден.");
                return;
            }

            using var temp = new FileWrapper(path, true);
            var words = temp.ReadAllWords();
            var sorted = words.OrderBy(w => w.Length).ThenBy(w => w).ToList();

            using var writer = new FileWrapper(path, false);
            writer.RewriteFile(sorted);

            Console.WriteLine($"{path}: отсортировано по длине слов.");

            // Переоткрываем основной объект, если он был
            wrapper?.Dispose();
            wrapper = new FileWrapper(path, false);
        }

        static void CrossReplaceWords()
        {
            var words1 = file1.ReadAllWords();
            var words2 = file2.ReadAllWords();

            if (words1.Count == 0 || words2.Count == 0)
            {
                Console.WriteLine("Один из файлов пуст!");
                return;
            }

            var new1 = new List<string>(words1);
            var new2 = new List<string>(words2);

            // Чётные слова в 1-м файле ← из 2-го (по кругу)
            for (int i = 1; i < words1.Count; i += 2)
                new1[i] = words2[i % words2.Count];

            // Нечётные слова во 2-м файле ← из 1-го (по кругу)
            for (int i = 0; i < words2.Count; i += 2)
                new2[i] = words1[i % words1.Count];

            // Записываем обратно
            using var file1Writer = new FileWrapper("21.txt", false);
            file1Writer.RewriteFile(new1);

            using var file2Writer = new FileWrapper("22.txt", false);
            file2Writer.RewriteFile(new2);
        }
    }
}