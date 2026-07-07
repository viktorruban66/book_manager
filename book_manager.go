// book_manager.go
package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"os"
	"strconv"
	"strings"
	"time"
)

type Book struct {
	ID        int    `json:"id"`
	Title     string `json:"title"`
	Author    string `json:"author"`
	Year      int    `json:"year"`
	Genre     string `json:"genre"`
	Read      bool   `json:"read"`
	Rating    int    `json:"rating"`
	AddedDate string `json:"added_date"`
}

type BooksData struct {
	Books []Book `json:"books"`
}

type BookManager struct {
	books  []Book
	nextID int
}

func NewBookManager() *BookManager {
	return &BookManager{
		books:  []Book{},
		nextID: 1,
	}
}

func (m *BookManager) AddBook(title, author string, year int, genre string, read bool, rating int) (Book, error) {
	if rating < 1 || rating > 10 {
		return Book{}, fmt.Errorf("оценка должна быть от 1 до 10")
	}
	if year < 0 || year > time.Now().Year() {
		return Book{}, fmt.Errorf("год должен быть от 0 до %d", time.Now().Year())
	}
	book := Book{
		ID:        m.nextID,
		Title:     title,
		Author:    author,
		Year:      year,
		Genre:     genre,
		Read:      read,
		Rating:    rating,
		AddedDate: time.Now().Format("2006-01-02"),
	}
	m.books = append(m.books, book)
	m.nextID++
	return book, nil
}

func (m *BookManager) FindBook(id int) *Book {
	for i := range m.books {
		if m.books[i].ID == id {
			return &m.books[i]
		}
	}
	return nil
}

func (m *BookManager) EditBook(id int, updates map[string]interface{}) bool {
	book := m.FindBook(id)
	if book == nil {
		return false
	}
	for key, value := range updates {
		switch key {
		case "title":
			if v, ok := value.(string); ok {
				book.Title = v
			}
		case "author":
			if v, ok := value.(string); ok {
				book.Author = v
			}
		case "year":
			if v, ok := value.(int); ok {
				book.Year = v
			}
		case "genre":
			if v, ok := value.(string); ok {
				book.Genre = v
			}
		case "read":
			if v, ok := value.(bool); ok {
				book.Read = v
			}
		case "rating":
			if v, ok := value.(int); ok {
				book.Rating = v
			}
		}
	}
	return true
}

func (m *BookManager) DeleteBook(id int) bool {
	for i, b := range m.books {
		if b.ID == id {
			m.books = append(m.books[:i], m.books[i+1:]...)
			return true
		}
	}
	return false
}

func (m *BookManager) SearchBooks(query string) []Book {
	q := strings.ToLower(query)
	var result []Book
	for _, b := range m.books {
		if strings.Contains(strings.ToLower(b.Title), q) || strings.Contains(strings.ToLower(b.Author), q) {
			result = append(result, b)
		}
	}
	return result
}

func (m *BookManager) FilterByRead(read bool) []Book {
	var result []Book
	for _, b := range m.books {
		if b.Read == read {
			result = append(result, b)
		}
	}
	return result
}

func (m *BookManager) FilterByGenre(genre string) []Book {
	var result []Book
	for _, b := range m.books {
		if strings.EqualFold(b.Genre, genre) {
			result = append(result, b)
		}
	}
	return result
}

func (m *BookManager) GetStats() map[string]interface{} {
	total := len(m.books)
	read := len(m.FilterByRead(true))
	unread := total - read
	var ratings []int
	for _, b := range m.books {
		if b.Read {
			ratings = append(ratings, b.Rating)
		}
	}
	avgRating := 0.0
	if len(ratings) > 0 {
		sum := 0
		for _, r := range ratings {
			sum += r
		}
		avgRating = float64(sum) / float64(len(ratings))
	}
	genres := make(map[string]int)
	for _, b := range m.books {
		genres[b.Genre]++
	}
	return map[string]interface{}{
		"total":       total,
		"read":        read,
		"unread":      unread,
		"avg_rating":  avgRating,
		"genres":      genres,
	}
}

func (m *BookManager) SaveToFile(filename string) error {
	data := BooksData{Books: m.books}
	jsonData, err := json.MarshalIndent(data, "", "  ")
	if err != nil {
		return err
	}
	return os.WriteFile(filename, jsonData, 0644)
}

func (m *BookManager) LoadFromFile(filename string) error {
	data, err := os.ReadFile(filename)
	if err != nil {
		if os.IsNotExist(err) {
			return nil
		}
		return err
	}
	var bd BooksData
	if err := json.Unmarshal(data, &bd); err != nil {
		return err
	}
	m.books = bd.Books
	for _, b := range m.books {
		if b.ID >= m.nextID {
			m.nextID = b.ID + 1
		}
	}
	return nil
}

func readString(prompt string) string {
	fmt.Print(prompt)
	reader := bufio.NewReader(os.Stdin)
	input, _ := reader.ReadString('\n')
	return strings.TrimSpace(input)
}

func readInt(prompt string) int {
	for {
		input := readString(prompt)
		if val, err := strconv.Atoi(input); err == nil {
			return val
		}
		fmt.Println("Введите число.")
	}
}

func readBool(prompt string) bool {
	for {
		input := readString(prompt)
		if input == "1" {
			return true
		} else if input == "0" {
			return false
		}
		fmt.Println("Введите 1 или 0.")
	}
}

func printBook(book Book) {
	status := "✅ Прочитана"
	if !book.Read {
		status = "📖 Не прочитана"
	}
	fmt.Printf("#%d - %s (%d)\n", book.ID, book.Title, book.Year)
	fmt.Printf("   Автор: %s, Жанр: %s\n", book.Author, book.Genre)
	fmt.Printf("   %s, Оценка: %d/10\n", status, book.Rating)
	fmt.Printf("   Добавлена: %s\n", book.AddedDate)
}

func main() {
	manager := NewBookManager()
	if err := manager.LoadFromFile("books_data.json"); err != nil {
		fmt.Println("Ошибка загрузки:", err)
	}

	for {
		fmt.Println("\n===== МЕНЕДЖЕР КНИГ (Go) =====")
		fmt.Println("1. Добавить книгу")
		fmt.Println("2. Показать все книги")
		fmt.Println("3. Показать прочитанные книги")
		fmt.Println("4. Показать непрочитанные книги")
		fmt.Println("5. Найти книги по автору или названию")
		fmt.Println("6. Редактировать книгу")
		fmt.Println("7. Удалить книгу")
		fmt.Println("8. Показать статистику")
		fmt.Println("9. Сохранить в файл")
		fmt.Println("10. Загрузить из файла")
		fmt.Println("0. Выход")
		choice := readString("Выберите действие: ")

		switch choice {
		case "0":
			return
		case "1":
			title := readString("Название: ")
			if title == "" {
				fmt.Println("Название не может быть пустым.")
				continue
			}
			author := readString("Автор: ")
			if author == "" {
				fmt.Println("Автор не может быть пустым.")
				continue
			}
			year := readInt("Год издания: ")
			genre := readString("Жанр: ")
			if genre == "" {
				genre = "Неизвестно"
			}
			read := readBool("Статус (1-прочитана, 0-не прочитана): ")
			rating := readInt("Оценка (1-10): ")
			book, err := manager.AddBook(title, author, year, genre, read, rating)
			if err != nil {
				fmt.Println("Ошибка:", err)
			} else {
				fmt.Printf("Книга добавлена с ID %d\n", book.ID)
			}
		case "2":
			if len(manager.books) == 0 {
				fmt.Println("Нет книг.")
			} else {
				for _, b := range manager.books {
					printBook(b)
				}
			}
		case "3":
			books := manager.FilterByRead(true)
			if len(books) == 0 {
				fmt.Println("Нет прочитанных книг.")
			} else {
				for _, b := range books {
					printBook(b)
				}
			}
		case "4":
			books := manager.FilterByRead(false)
			if len(books) == 0 {
				fmt.Println("Нет непрочитанных книг.")
			} else {
				for _, b := range books {
					printBook(b)
				}
			}
		case "5":
			query := readString("Введите часть названия или автора: ")
			results := manager.SearchBooks(query)
			if len(results) == 0 {
				fmt.Println("Книги не найдены.")
			} else {
				for _, b := range results {
					printBook(b)
				}
			}
		case "6":
			id := readInt("Введите ID книги для редактирования: ")
			book := manager.FindBook(id)
			if book == nil {
				fmt.Println("Книга не найдена.")
				continue
			}
			fmt.Println("Оставьте поле пустым, чтобы не менять.")
			newTitle := readString(fmt.Sprintf("Название (%s): ", book.Title))
			newAuthor := readString(fmt.Sprintf("Автор (%s): ", book.Author))
			newYear := readString(fmt.Sprintf("Год (%d): ", book.Year))
			newGenre := readString(fmt.Sprintf("Жанр (%s): ", book.Genre))
			newRead := readString(fmt.Sprintf("Статус (1-прочитана, 0-не прочитана) сейчас: %d: ", map[bool]int{true: 1, false: 0}[book.Read]))
			newRating := readString(fmt.Sprintf("Оценка (%d): ", book.Rating))
			updates := make(map[string]interface{})
			if newTitle != "" {
				updates["title"] = newTitle
			}
			if newAuthor != "" {
				updates["author"] = newAuthor
			}
			if newYear != "" {
				if y, err := strconv.Atoi(newYear); err == nil {
					updates["year"] = y
				} else {
					fmt.Println("Год должен быть числом, пропускаем.")
				}
			}
			if newGenre != "" {
				updates["genre"] = newGenre
			}
			if newRead != "" {
				updates["read"] = newRead == "1"
			}
			if newRating != "" {
				if r, err := strconv.Atoi(newRating); err == nil {
					updates["rating"] = r
				} else {
					fmt.Println("Оценка должна быть числом, пропускаем.")
				}
			}
			if manager.EditBook(id, updates) {
				fmt.Println("Книга обновлена.")
			} else {
				fmt.Println("Ошибка обновления.")
			}
		case "7":
			id := readInt("Введите ID книги для удаления: ")
			if manager.DeleteBook(id) {
				fmt.Println("Книга удалена.")
			} else {
				fmt.Println("Книга не найдена.")
			}
		case "8":
			stats := manager.GetStats()
			fmt.Println("\n=== СТАТИСТИКА ===")
			fmt.Printf("Всего книг: %d\n", stats["total"])
			fmt.Printf("Прочитано: %d\n", stats["read"])
			fmt.Printf("Не прочитано: %d\n", stats["unread"])
			fmt.Printf("Средняя оценка: %.2f\n", stats["avg_rating"])
			fmt.Println("По жанрам:")
			genres := stats["genres"].(map[string]int)
			for g, c := range genres {
				fmt.Printf("  %s: %d\n", g, c)
			}
		case "9":
			if err := manager.SaveToFile("books_data.json"); err != nil {
				fmt.Println("Ошибка сохранения:", err)
			} else {
				fmt.Println("Сохранено.")
			}
		case "10":
			if err := manager.LoadFromFile("books_data.json"); err != nil {
				fmt.Println("Ошибка загрузки:", err)
			} else {
				fmt.Println("Загружено.")
			}
		default:
			fmt.Println("Неизвестная команда.")
		}
	}
}
