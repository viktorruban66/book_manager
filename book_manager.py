# book_manager.py
import json
from dataclasses import dataclass, asdict
from datetime import datetime, date
from typing import List, Optional
from pathlib import Path

@dataclass
class Book:
    id: int
    title: str
    author: str
    year: int
    genre: str
    read: bool
    rating: int  # 1-10
    added_date: str

class BookManager:
    def __init__(self):
        self.books: List[Book] = []
        self.next_id = 1

    def add_book(self, title: str, author: str, year: int, genre: str, read: bool, rating: int) -> Book:
        if rating < 1 or rating > 10:
            raise ValueError("Оценка должна быть от 1 до 10")
        if year < 0 or year > datetime.now().year:
            raise ValueError(f"Год должен быть от 0 до {datetime.now().year}")
        book = Book(
            id=self.next_id,
            title=title,
            author=author,
            year=year,
            genre=genre,
            read=read,
            rating=rating,
            added_date=date.today().isoformat()
        )
        self.books.append(book)
        self.next_id += 1
        return book

    def find_book(self, book_id: int) -> Optional[Book]:
        return next((b for b in self.books if b.id == book_id), None)

    def edit_book(self, book_id: int, **kwargs) -> bool:
        book = self.find_book(book_id)
        if not book:
            return False
        for key, value in kwargs.items():
            if hasattr(book, key) and value is not None:
                setattr(book, key, value)
        return True

    def delete_book(self, book_id: int) -> bool:
        book = self.find_book(book_id)
        if book:
            self.books.remove(book)
            return True
        return False

    def search_books(self, query: str) -> List[Book]:
        q = query.lower()
        return [b for b in self.books if q in b.title.lower() or q in b.author.lower()]

    def filter_by_read(self, read: bool) -> List[Book]:
        return [b for b in self.books if b.read == read]

    def filter_by_genre(self, genre: str) -> List[Book]:
        return [b for b in self.books if b.genre.lower() == genre.lower()]

    def get_stats(self) -> dict:
        total = len(self.books)
        read = len(self.filter_by_read(True))
        unread = total - read
        ratings = [b.rating for b in self.books if b.read]
        avg_rating = sum(ratings) / len(ratings) if ratings else 0
        genres = {}
        for b in self.books:
            genres[b.genre] = genres.get(b.genre, 0) + 1
        return {
            "total": total,
            "read": read,
            "unread": unread,
            "avg_rating": avg_rating,
            "genres": genres
        }

    def save_to_file(self, filename: str = "books_data.json") -> None:
        data = {"books": [asdict(b) for b in self.books]}
        with open(filename, "w", encoding="utf-8") as f:
            json.dump(data, f, ensure_ascii=False, indent=2)

    def load_from_file(self, filename: str = "books_data.json") -> None:
        path = Path(filename)
        if not path.exists():
            return
        with open(filename, "r", encoding="utf-8") as f:
            data = json.load(f)
            self.books.clear()
            for item in data.get("books", []):
                book = Book(
                    id=item["id"],
                    title=item["title"],
                    author=item["author"],
                    year=item["year"],
                    genre=item["genre"],
                    read=item["read"],
                    rating=item["rating"],
                    added_date=item["added_date"]
                )
                self.books.append(book)
                if book.id >= self.next_id:
                    self.next_id = book.id + 1

def print_book(book: Book) -> None:
    status = "✅ Прочитана" if book.read else "📖 Не прочитана"
    print(f"#{book.id} - {book.title} ({book.year})")
    print(f"   Автор: {book.author}, Жанр: {book.genre}")
    print(f"   {status}, Оценка: {book.rating}/10")
    print(f"   Добавлена: {book.added_date}")

def main():
    manager = BookManager()
    manager.load_from_file()

    while True:
        print("\n===== МЕНЕДЖЕР КНИГ (Python) =====")
        print("1. Добавить книгу")
        print("2. Показать все книги")
        print("3. Показать прочитанные книги")
        print("4. Показать непрочитанные книги")
        print("5. Найти книги по автору или названию")
        print("6. Редактировать книгу")
        print("7. Удалить книгу")
        print("8. Показать статистику")
        print("9. Сохранить в файл")
        print("10. Загрузить из файла")
        print("0. Выход")
        choice = input("Выберите действие: ").strip()

        if choice == "0":
            break
        elif choice == "1":
            title = input("Название: ").strip()
            if not title:
                print("Название не может быть пустым.")
                continue
            author = input("Автор: ").strip()
            if not author:
                print("Автор не может быть пустым.")
                continue
            try:
                year = int(input("Год издания: ").strip())
            except ValueError:
                print("Введите число.")
                continue
            genre = input("Жанр: ").strip()
            if not genre:
                genre = "Неизвестно"
            read_input = input("Статус (1-прочитана, 0-не прочитана): ").strip()
            read = read_input == "1"
            try:
                rating = int(input("Оценка (1-10): ").strip())
            except ValueError:
                rating = 0
            try:
                book = manager.add_book(title, author, year, genre, read, rating)
                print(f"Книга добавлена с ID {book.id}")
            except Exception as e:
                print("Ошибка:", e)
        elif choice == "2":
            if not manager.books:
                print("Нет книг.")
            else:
                for b in manager.books:
                    print_book(b)
        elif choice == "3":
            books = manager.filter_by_read(True)
            if not books:
                print("Нет прочитанных книг.")
            else:
                for b in books:
                    print_book(b)
        elif choice == "4":
            books = manager.filter_by_read(False)
            if not books:
                print("Нет непрочитанных книг.")
            else:
                for b in books:
                    print_book(b)
        elif choice == "5":
            query = input("Введите часть названия или автора: ").strip()
            if not query:
                print("Введите текст.")
                continue
            results = manager.search_books(query)
            if not results:
                print("Книги не найдены.")
            else:
                for b in results:
                    print_book(b)
        elif choice == "6":
            try:
                bid = int(input("Введите ID книги для редактирования: ").strip())
            except ValueError:
                print("Некорректный ID.")
                continue
            book = manager.find_book(bid)
            if not book:
                print("Книга не найдена.")
                continue
            print("Оставьте поле пустым, чтобы не менять.")
            new_title = input(f"Название ({book.title}): ").strip()
            new_author = input(f"Автор ({book.author}): ").strip()
            new_year = input(f"Год ({book.year}): ").strip()
            new_genre = input(f"Жанр ({book.genre}): ").strip()
            new_read = input(f"Статус (1-прочитана, 0-не прочитана) сейчас: {'1' if book.read else '0'}: ").strip()
            new_rating = input(f"Оценка ({book.rating}): ").strip()
            updates = {}
            if new_title: updates["title"] = new_title
            if new_author: updates["author"] = new_author
            if new_year:
                try:
                    updates["year"] = int(new_year)
                except ValueError:
                    print("Год должен быть числом, пропускаем.")
            if new_genre: updates["genre"] = new_genre
            if new_read: updates["read"] = new_read == "1"
            if new_rating:
                try:
                    updates["rating"] = int(new_rating)
                except ValueError:
                    print("Оценка должна быть числом, пропускаем.")
            if manager.edit_book(bid, **updates):
                print("Книга обновлена.")
            else:
                print("Ошибка обновления.")
        elif choice == "7":
            try:
                bid = int(input("Введите ID книги для удаления: ").strip())
            except ValueError:
                print("Некорректный ID.")
                continue
            if manager.delete_book(bid):
                print("Книга удалена.")
            else:
                print("Книга не найдена.")
        elif choice == "8":
            stats = manager.get_stats()
            print("\n=== СТАТИСТИКА ===")
            print(f"Всего книг: {stats['total']}")
            print(f"Прочитано: {stats['read']}")
            print(f"Не прочитано: {stats['unread']}")
            print(f"Средняя оценка: {stats['avg_rating']:.2f}")
            print("По жанрам:")
            for genre, count in stats['genres'].items():
                print(f"  {genre}: {count}")
        elif choice == "9":
            manager.save_to_file()
            print("Сохранено.")
        elif choice == "10":
            manager.load_from_file()
            print("Загружено.")
        else:
            print("Неизвестная команда.")

if __name__ == "__main__":
    main()
