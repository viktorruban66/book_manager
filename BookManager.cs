// BookManager.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Text.Json.Serialization;

public record Book(
    int Id,
    string Title,
    string Author,
    int Year,
    string Genre,
    bool Read,
    int Rating,
    string AddedDate
);

public class BooksData
{
    public List<Book> Books { get; set; } = new();
}

public class BookManager
{
    private List<Book> books = new();
    private int nextId = 1;

    public IReadOnlyList<Book> Books => books.AsReadOnly();

    public Book AddBook(string title, string author, int year, string genre, bool read, int rating)
    {
        if (rating < 1 || rating > 10) throw new ArgumentException("Оценка должна быть от 1 до 10");
        if (year < 0 || year > DateTime.Now.Year)
            throw new ArgumentException($"Год должен быть от 0 до {DateTime.Now.Year}");
        var book = new Book(nextId, title, author, year, genre, read, rating, DateTime.Now.ToString("yyyy-MM-dd"));
        books.Add(book);
        nextId++;
        return book;
    }

    public Book? FindBook(int id) => books.FirstOrDefault(b => b.Id == id);

    public bool EditBook(int id, Dictionary<string, object> updates)
    {
        var old = FindBook(id);
        if (old == null) return false;
        books.Remove(old);
        string title = updates.ContainsKey("title") ? (string)updates["title"] : old.Title;
        string author = updates.ContainsKey("author") ? (string)updates["author"] : old.Author;
        int year = updates.ContainsKey("year") ? (int)updates["year"] : old.Year;
        string genre = updates.ContainsKey("genre") ? (string)updates["genre"] : old.Genre;
        bool read = updates.ContainsKey("read") ? (bool)updates["read"] : old.Read;
        int rating = updates.ContainsKey("rating") ? (int)updates["rating"] : old.Rating;
        var updated = new Book(old.Id, title, author, year, genre, read, rating, old.AddedDate);
        books.Add(updated);
        return true;
    }

    public bool DeleteBook(int id) => books.RemoveAll(b => b.Id == id) > 0;

    public List<Book> SearchBooks(string query)
    {
        var q = query.ToLower();
        return books.Where(b => b.Title.ToLower().Contains(q) || b.Author.ToLower().Contains(q)).ToList();
    }

    public List<Book> FilterByRead(bool read) => books.Where(b => b.Read == read).ToList();

    public List<Book> FilterByGenre(string genre) =>
        books.Where(b => string.Equals(b.Genre, genre, StringComparison.OrdinalIgnoreCase)).ToList();

    public Dictionary<string, object> GetStats()
    {
        int total = books.Count;
        int readCount = FilterByRead(true).Count;
        int unread = total - readCount;
        double avgRating = FilterByRead(true).Any() ? FilterByRead(true).Average(b => b.Rating) : 0;
        var genres = books.GroupBy(b => b.Genre).ToDictionary(g => g.Key, g => g.Count());
        return new Dictionary<string, object>
        {
            ["total"] = total,
            ["read"] = readCount,
            ["unread"] = unread,
            ["avg_rating"] = avgRating,
            ["genres"] = genres
        };
    }

    public void SaveToFile(string filename)
    {
        var data = new BooksData { Books = books };
        var options = new JsonSerializerOptions { WriteIndented = true };
        string json = JsonSerializer.Serialize(data, options);
        File.WriteAllText(filename, json);
    }

    public void LoadFromFile(string filename)
    {
        if (!File.Exists(filename)) return;
        string json = File.ReadAllText(filename);
        var data = JsonSerializer.Deserialize<BooksData>(json);
        if (data != null)
        {
            books = data.Books;
            nextId = books.Any() ? books.Max(b => b.Id) + 1 : 1;
        }
    }
}

public static class Program
{
    private static string ReadString(string prompt)
    {
        Console.Write(prompt);
        return Console.ReadLine()?.Trim() ?? "";
    }

    private static int ReadInt(string prompt)
    {
        while (true)
        {
            Console.Write(prompt);
            if (int.TryParse(Console.ReadLine(), out int result))
                return result;
            Console.WriteLine("Введите число.");
        }
    }

    private static bool ReadBool(string prompt)
    {
        while (true)
        {
            string input = ReadString(prompt);
            if (input == "1") return true;
            if (input == "0") return false;
            Console.WriteLine("Введите 1 или 0.");
        }
    }

    private static void PrintBook(Book book)
    {
        string status = book.Read ? "✅ Прочитана" : "📖 Не прочитана";
        Console.WriteLine($"#{book.Id} - {book.Title} ({book.Year})");
        Console.WriteLine($"   Автор: {book.Author}, Жанр: {book.Genre}");
        Console.WriteLine($"   {status}, Оценка: {book.Rating}/10");
        Console.WriteLine($"   Добавлена: {book.AddedDate}");
    }

    public static void Main()
    {
        var manager = new BookManager();
        try { manager.LoadFromFile("books_data.json"); }
        catch { Console.WriteLine("Не удалось загрузить данные."); }

        while (true)
        {
            Console.WriteLine("\n===== МЕНЕДЖЕР КНИГ (C#) =====");
            Console.WriteLine("1. Добавить книгу");
            Console.WriteLine("2. Показать все книги");
            Console.WriteLine("3. Показать прочитанные книги");
            Console.WriteLine("4. Показать непрочитанные книги");
            Console.WriteLine("5. Найти книги по автору или названию");
            Console.WriteLine("6. Редактировать книгу");
            Console.WriteLine("7. Удалить книгу");
            Console.WriteLine("8. Показать статистику");
            Console.WriteLine("9. Сохранить в файл");
            Console.WriteLine("10. Загрузить из файла");
            Console.WriteLine("0. Выход");
            string choice = ReadString("Выберите действие: ");

            switch (choice)
            {
                case "0": return;
                case "1":
                    string title = ReadString("Название: ");
                    if (string.IsNullOrWhiteSpace(title)) { Console.WriteLine("Название не может быть пустым."); continue; }
                    string author = ReadString("Автор: ");
                    if (string.IsNullOrWhiteSpace(author)) { Console.WriteLine("Автор не может быть пустым."); continue; }
                    int year = ReadInt("Год издания: ");
                    string genre = ReadString("Жанр: ");
                    if (string.IsNullOrWhiteSpace(genre)) genre = "Неизвестно";
                    bool read = ReadBool("Статус (1-прочитана, 0-не прочитана): ");
                    int rating = ReadInt("Оценка (1-10): ");
                    try
                    {
                        var book = manager.AddBook(title, author, year, genre, read, rating);
                        Console.WriteLine($"Книга добавлена с ID {book.Id}");
                    }
                    catch (Exception ex) { Console.WriteLine($"Ошибка: {ex.Message}"); }
                    break;
                case "2":
                    if (!manager.Books.Any()) Console.WriteLine("Нет книг.");
                    else foreach (var b in manager.Books) PrintBook(b);
                    break;
                case "3":
                    var readBooks = manager.FilterByRead(true);
                    if (!readBooks.Any()) Console.WriteLine("Нет прочитанных книг.");
                    else foreach (var b in readBooks) PrintBook(b);
                    break;
                case "4":
                    var unreadBooks = manager.FilterByRead(false);
                    if (!unreadBooks.Any()) Console.WriteLine("Нет непрочитанных книг.");
                    else foreach (var b in unreadBooks) PrintBook(b);
                    break;
                case "5":
                    string query = ReadString("Введите часть названия или автора: ");
                    var results = manager.SearchBooks(query);
                    if (!results.Any()) Console.WriteLine("Книги не найдены.");
                    else foreach (var b in results) PrintBook(b);
                    break;
                case "6":
                    int id = ReadInt("Введите ID книги для редактирования: ");
                    var old = manager.FindBook(id);
                    if (old == null) { Console.WriteLine("Книга не найдена."); continue; }
                    Console.WriteLine("Оставьте поле пустым, чтобы не менять.");
                    string newTitle = ReadString($"Название ({old.Title}): ");
                    string newAuthor = ReadString($"Автор ({old.Author}): ");
                    string newYearStr = ReadString($"Год ({old.Year}): ");
                    string newGenre = ReadString($"Жанр ({old.Genre}): ");
                    string newReadStr = ReadString($"Статус (1-прочитана, 0-не прочитана) сейчас: {(old.Read ? "1" : "0")}: ");
                    string newRatingStr = ReadString($"Оценка ({old.Rating}): ");
                    var updates = new Dictionary<string, object>();
                    if (!string.IsNullOrWhiteSpace(newTitle)) updates["title"] = newTitle;
                    if (!string.IsNullOrWhiteSpace(newAuthor)) updates["author"] = newAuthor;
                    if (!string.IsNullOrWhiteSpace(newYearStr))
                    {
                        if (int.TryParse(newYearStr, out int y)) updates["year"] = y;
                        else Console.WriteLine("Год должен быть числом, пропускаем.");
                    }
                    if (!string.IsNullOrWhiteSpace(newGenre)) updates["genre"] = newGenre;
                    if (!string.IsNullOrWhiteSpace(newReadStr)) updates["read"] = newReadStr == "1";
                    if (!string.IsNullOrWhiteSpace(newRatingStr))
                    {
                        if (int.TryParse(newRatingStr, out int r)) updates["rating"] = r;
                        else Console.WriteLine("Оценка должна быть числом, пропускаем.");
                    }
                    if (manager.EditBook(id, updates)) Console.WriteLine("Книга обновлена.");
                    else Console.WriteLine("Ошибка обновления.");
                    break;
                case "7":
                    int delId = ReadInt("Введите ID книги для удаления: ");
                    if (manager.DeleteBook(delId)) Console.WriteLine("Книга удалена.");
                    else Console.WriteLine("Книга не найдена.");
                    break;
                case "8":
                    var stats = manager.GetStats();
                    Console.WriteLine("\n=== СТАТИСТИКА ===");
                    Console.WriteLine($"Всего книг: {stats["total"]}");
                    Console.WriteLine($"Прочитано: {stats["read"]}");
                    Console.WriteLine($"Не прочитано: {stats["unread"]}");
                    Console.WriteLine($"Средняя оценка: {stats["avg_rating"]:F2}");
                    Console.WriteLine("По жанрам:");
                    var genres = (Dictionary<string, int>)stats["genres"];
                    foreach (var kv in genres) Console.WriteLine($"  {kv.Key}: {kv.Value}");
                    break;
                case "9":
                    try { manager.SaveToFile("books_data.json"); Console.WriteLine("Сохранено."); }
                    catch (Exception ex) { Console.WriteLine($"Ошибка: {ex.Message}"); }
                    break;
                case "10":
                    try { manager.LoadFromFile("books_data.json"); Console.WriteLine("Загружено."); }
                    catch (Exception ex) { Console.WriteLine($"Ошибка: {ex.Message}"); }
                    break;
                default: Console.WriteLine("Неизвестная команда."); break;
            }
        }
    }
}
