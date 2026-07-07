// BookManager.java
import java.io.*;
import java.nio.file.*;
import java.time.LocalDate;
import java.util.*;
import java.util.stream.Collectors;

record Book(int id, String title, String author, int year, String genre, boolean read, int rating, String addedDate) implements Serializable {}

class BooksData implements Serializable {
    private static final long serialVersionUID = 1L;
    public List<Book> books;
}

class BookManager implements Serializable {
    private static final long serialVersionUID = 1L;
    private List<Book> books = new ArrayList<>();
    private int nextId = 1;

    public Book addBook(String title, String author, int year, String genre, boolean read, int rating) {
        if (rating < 1 || rating > 10) throw new IllegalArgumentException("Оценка должна быть от 1 до 10");
        if (year < 0 || year > LocalDate.now().getYear())
            throw new IllegalArgumentException("Год должен быть от 0 до " + LocalDate.now().getYear());
        Book book = new Book(nextId, title, author, year, genre, read, rating, LocalDate.now().toString());
        books.add(book);
        nextId++;
        return book;
    }

    public Optional<Book> findBook(int id) {
        return books.stream().filter(b -> b.id() == id).findFirst();
    }

    public boolean editBook(int id, Map<String, Object> updates) {
        Optional<Book> opt = findBook(id);
        if (opt.isEmpty()) return false;
        Book old = opt.get();
        books.remove(old);
        String title = (String) updates.getOrDefault("title", old.title());
        String author = (String) updates.getOrDefault("author", old.author());
        int year = (int) updates.getOrDefault("year", old.year());
        String genre = (String) updates.getOrDefault("genre", old.genre());
        boolean read = (boolean) updates.getOrDefault("read", old.read());
        int rating = (int) updates.getOrDefault("rating", old.rating());
        Book updated = new Book(old.id(), title, author, year, genre, read, rating, old.addedDate());
        books.add(updated);
        return true;
    }

    public boolean deleteBook(int id) {
        return books.removeIf(b -> b.id() == id);
    }

    public List<Book> searchBooks(String query) {
        String q = query.toLowerCase();
        return books.stream()
                .filter(b -> b.title().toLowerCase().contains(q) || b.author().toLowerCase().contains(q))
                .collect(Collectors.toList());
    }

    public List<Book> filterByRead(boolean read) {
        return books.stream().filter(b -> b.read() == read).collect(Collectors.toList());
    }

    public List<Book> filterByGenre(String genre) {
        return books.stream().filter(b -> b.genre().equalsIgnoreCase(genre)).collect(Collectors.toList());
    }

    public Map<String, Object> getStats() {
        int total = books.size();
        int readCount = filterByRead(true).size();
        int unread = total - readCount;
        double avgRating = filterByRead(true).stream().mapToInt(Book::rating).average().orElse(0);
        Map<String, Integer> genres = new HashMap<>();
        books.forEach(b -> genres.put(b.genre(), genres.getOrDefault(b.genre(), 0) + 1));
        Map<String, Object> stats = new HashMap<>();
        stats.put("total", total);
        stats.put("read", readCount);
        stats.put("unread", unread);
        stats.put("avg_rating", avgRating);
        stats.put("genres", genres);
        return stats;
    }

    public void saveToFile(String filename) throws IOException {
        BooksData data = new BooksData();
        data.books = new ArrayList<>(books);
        try (ObjectOutputStream oos = new ObjectOutputStream(Files.newOutputStream(Paths.get(filename)))) {
            oos.writeObject(data);
        }
    }

    public void loadFromFile(String filename) throws IOException, ClassNotFoundException {
        try (ObjectInputStream ois = new ObjectInputStream(Files.newInputStream(Paths.get(filename)))) {
            BooksData data = (BooksData) ois.readObject();
            books = new ArrayList<>(data.books);
            for (Book b : books) {
                if (b.id() >= nextId) nextId = b.id() + 1;
            }
        }
    }

    public List<Book> getBooks() { return Collections.unmodifiableList(books); }
}

public class BookManagerApp {
    private static final Scanner scanner = new Scanner(System.in);

    private static String readString(String prompt) {
        System.out.print(prompt);
        return scanner.nextLine().trim();
    }

    private static int readInt(String prompt) {
        while (true) {
            try {
                System.out.print(prompt);
                return Integer.parseInt(scanner.nextLine().trim());
            } catch (NumberFormatException e) {
                System.out.println("Введите число.");
            }
        }
    }

    private static boolean readBool(String prompt) {
        while (true) {
            String input = readString(prompt);
            if (input.equals("1")) return true;
            if (input.equals("0")) return false;
            System.out.println("Введите 1 или 0.");
        }
    }

    private static void printBook(Book book) {
        String status = book.read() ? "✅ Прочитана" : "📖 Не прочитана";
        System.out.printf("#%d - %s (%d)%n", book.id(), book.title(), book.year());
        System.out.printf("   Автор: %s, Жанр: %s%n", book.author(), book.genre());
        System.out.printf("   %s, Оценка: %d/10%n", status, book.rating());
        System.out.printf("   Добавлена: %s%n", book.addedDate());
    }

    public static void main(String[] args) {
        BookManager manager = new BookManager();
        try {
            manager.loadFromFile("books_data.ser");
        } catch (IOException | ClassNotFoundException e) {
            System.out.println("Не удалось загрузить данные.");
        }

        while (true) {
            System.out.println("\n===== МЕНЕДЖЕР КНИГ (Java) =====");
            System.out.println("1. Добавить книгу");
            System.out.println("2. Показать все книги");
            System.out.println("3. Показать прочитанные книги");
            System.out.println("4. Показать непрочитанные книги");
            System.out.println("5. Найти книги по автору или названию");
            System.out.println("6. Редактировать книгу");
            System.out.println("7. Удалить книгу");
            System.out.println("8. Показать статистику");
            System.out.println("9. Сохранить в файл");
            System.out.println("10. Загрузить из файла");
            System.out.println("0. Выход");
            String choice = readString("Выберите действие: ");

            switch (choice) {
                case "0" -> { return; }
                case "1" -> {
                    String title = readString("Название: ");
                    if (title.isBlank()) { System.out.println("Название не может быть пустым."); continue; }
                    String author = readString("Автор: ");
                    if (author.isBlank()) { System.out.println("Автор не может быть пустым."); continue; }
                    int year = readInt("Год издания: ");
                    String genre = readString("Жанр: ");
                    if (genre.isBlank()) genre = "Неизвестно";
                    boolean read = readBool("Статус (1-прочитана, 0-не прочитана): ");
                    int rating = readInt("Оценка (1-10): ");
                    try {
                        Book book = manager.addBook(title, author, year, genre, read, rating);
                        System.out.println("Книга добавлена с ID " + book.id());
                    } catch (Exception e) {
                        System.out.println("Ошибка: " + e.getMessage());
                    }
                }
                case "2" -> {
                    if (manager.getBooks().isEmpty()) System.out.println("Нет книг.");
                    else manager.getBooks().forEach(BookManagerApp::printBook);
                }
                case "3" -> {
                    var books = manager.filterByRead(true);
                    if (books.isEmpty()) System.out.println("Нет прочитанных книг.");
                    else books.forEach(BookManagerApp::printBook);
                }
                case "4" -> {
                    var books = manager.filterByRead(false);
                    if (books.isEmpty()) System.out.println("Нет непрочитанных книг.");
                    else books.forEach(BookManagerApp::printBook);
                }
                case "5" -> {
                    String query = readString("Введите часть названия или автора: ");
                    var results = manager.searchBooks(query);
                    if (results.isEmpty()) System.out.println("Книги не найдены.");
                    else results.forEach(BookManagerApp::printBook);
                }
                case "6" -> {
                    int id = readInt("Введите ID книги для редактирования: ");
                    var opt = manager.findBook(id);
                    if (opt.isEmpty()) { System.out.println("Книга не найдена."); continue; }
                    Book old = opt.get();
                    System.out.println("Оставьте поле пустым, чтобы не менять.");
                    String newTitle = readString("Название (" + old.title() + "): ");
                    String newAuthor = readString("Автор (" + old.author() + "): ");
                    String newYearStr = readString("Год (" + old.year() + "): ");
                    String newGenre = readString("Жанр (" + old.genre() + "): ");
                    String newReadStr = readString("Статус (1-прочитана, 0-не прочитана) сейчас: " + (old.read() ? "1" : "0") + ": ");
                    String newRatingStr = readString("Оценка (" + old.rating() + "): ");
                    Map<String, Object> updates = new HashMap<>();
                    if (!newTitle.isBlank()) updates.put("title", newTitle);
                    if (!newAuthor.isBlank()) updates.put("author", newAuthor);
                    if (!newYearStr.isBlank()) {
                        try { updates.put("year", Integer.parseInt(newYearStr)); }
                        catch (NumberFormatException e) { System.out.println("Год должен быть числом, пропускаем."); }
                    }
                    if (!newGenre.isBlank()) updates.put("genre", newGenre);
                    if (!newReadStr.isBlank()) updates.put("read", newReadStr.equals("1"));
                    if (!newRatingStr.isBlank()) {
                        try { updates.put("rating", Integer.parseInt(newRatingStr)); }
                        catch (NumberFormatException e) { System.out.println("Оценка должна быть числом, пропускаем."); }
                    }
                    if (manager.editBook(id, updates)) System.out.println("Книга обновлена.");
                    else System.out.println("Ошибка обновления.");
                }
                case "7" -> {
                    int id = readInt("Введите ID книги для удаления: ");
                    if (manager.deleteBook(id)) System.out.println("Книга удалена.");
                    else System.out.println("Книга не найдена.");
                }
                case "8" -> {
                    var stats = manager.getStats();
                    System.out.println("\n=== СТАТИСТИКА ===");
                    System.out.println("Всего книг: " + stats.get("total"));
                    System.out.println("Прочитано: " + stats.get("read"));
                    System.out.println("Не прочитано: " + stats.get("unread"));
                    System.out.printf("Средняя оценка: %.2f%n", stats.get("avg_rating"));
                    System.out.println("По жанрам:");
                    @SuppressWarnings("unchecked")
                    Map<String, Integer> genres = (Map<String, Integer>) stats.get("genres");
                    genres.forEach((g, c) -> System.out.println("  " + g + ": " + c));
                }
                case "9" -> {
                    try {
                        manager.saveToFile("books_data.ser");
                        System.out.println("Сохранено.");
                    } catch (IOException e) {
                        System.out.println("Ошибка сохранения: " + e.getMessage());
                    }
                }
                case "10" -> {
                    try {
                        manager.loadFromFile("books_data.ser");
                        System.out.println("Загружено.");
                    } catch (IOException | ClassNotFoundException e) {
                        System.out.println("Ошибка загрузки: " + e.getMessage());
                    }
                }
                default -> System.out.println("Неизвестная команда.");
            }
        }
    }
}
