// book_manager.js
const fs = require('fs').promises;
const readline = require('readline');

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

const question = (prompt) => new Promise(resolve => rl.question(prompt, resolve));

class Book {
    constructor(id, title, author, year, genre, read, rating, addedDate) {
        this.id = id;
        this.title = title;
        this.author = author;
        this.year = year;
        this.genre = genre;
        this.read = read;
        this.rating = rating;
        this.addedDate = addedDate || new Date().toISOString().slice(0, 10);
    }
}

class BookManager {
    constructor() {
        this.books = [];
        this.nextId = 1;
    }

    addBook(title, author, year, genre, read, rating) {
        if (rating < 1 || rating > 10) throw new Error('Оценка должна быть от 1 до 10');
        if (year < 0 || year > new Date().getFullYear())
            throw new Error(`Год должен быть от 0 до ${new Date().getFullYear()}`);
        const book = new Book(this.nextId, title, author, year, genre, read, rating);
        this.books.push(book);
        this.nextId++;
        return book;
    }

    findBook(id) {
        return this.books.find(b => b.id === id);
    }

    editBook(id, updates) {
        const book = this.findBook(id);
        if (!book) return false;
        Object.assign(book, updates);
        return true;
    }

    deleteBook(id) {
        const index = this.books.findIndex(b => b.id === id);
        if (index === -1) return false;
        this.books.splice(index, 1);
        return true;
    }

    searchBooks(query) {
        const q = query.toLowerCase();
        return this.books.filter(b => b.title.toLowerCase().includes(q) || b.author.toLowerCase().includes(q));
    }

    filterByRead(read) {
        return this.books.filter(b => b.read === read);
    }

    filterByGenre(genre) {
        return this.books.filter(b => b.genre.toLowerCase() === genre.toLowerCase());
    }

    getStats() {
        const total = this.books.length;
        const readCount = this.filterByRead(true).length;
        const unread = total - readCount;
        const ratings = this.filterByRead(true).map(b => b.rating);
        const avgRating = ratings.length ? ratings.reduce((a, b) => a + b, 0) / ratings.length : 0;
        const genres = {};
        this.books.forEach(b => { genres[b.genre] = (genres[b.genre] || 0) + 1; });
        return { total, read: readCount, unread, avgRating, genres };
    }

    async saveToFile(filename = 'books_data.json') {
        const data = { books: this.books };
        await fs.writeFile(filename, JSON.stringify(data, null, 2));
    }

    async loadFromFile(filename = 'books_data.json') {
        try {
            const data = await fs.readFile(filename, 'utf8');
            const parsed = JSON.parse(data);
            this.books = parsed.books.map(b => Object.assign(new Book(0), b));
            this.nextId = this.books.reduce((max, b) => Math.max(max, b.id), 0) + 1;
        } catch (err) {
            if (err.code !== 'ENOENT') throw err;
        }
    }
}

function printBook(book) {
    const status = book.read ? '✅ Прочитана' : '📖 Не прочитана';
    console.log(`#${book.id} - ${book.title} (${book.year})`);
    console.log(`   Автор: ${book.author}, Жанр: ${book.genre}`);
    console.log(`   ${status}, Оценка: ${book.rating}/10`);
    console.log(`   Добавлена: ${book.addedDate}`);
}

async function main() {
    const manager = new BookManager();
    await manager.loadFromFile();

    while (true) {
        console.log('\n===== МЕНЕДЖЕР КНИГ (JavaScript) =====');
        console.log('1. Добавить книгу');
        console.log('2. Показать все книги');
        console.log('3. Показать прочитанные книги');
        console.log('4. Показать непрочитанные книги');
        console.log('5. Найти книги по автору или названию');
        console.log('6. Редактировать книгу');
        console.log('7. Удалить книгу');
        console.log('8. Показать статистику');
        console.log('9. Сохранить в файл');
        console.log('10. Загрузить из файла');
        console.log('0. Выход');
        const choice = await question('Выберите действие: ');

        if (choice === '0') break;

        switch (choice) {
            case '1': {
                const title = await question('Название: ');
                if (!title.trim()) { console.log('Название не может быть пустым.'); continue; }
                const author = await question('Автор: ');
                if (!author.trim()) { console.log('Автор не может быть пустым.'); continue; }
                const year = parseInt(await question('Год издания: '));
                let genre = await question('Жанр: ');
                if (!genre.trim()) genre = 'Неизвестно';
                const read = (await question('Статус (1-прочитана, 0-не прочитана): ')) === '1';
                const rating = parseInt(await question('Оценка (1-10): '));
                try {
                    const book = manager.addBook(title, author, year, genre, read, rating);
                    console.log(`Книга добавлена с ID ${book.id}`);
                } catch (err) {
                    console.log('Ошибка:', err.message);
                }
                break;
            }
            case '2':
                if (manager.books.length === 0) console.log('Нет книг.');
                else manager.books.forEach(printBook);
                break;
            case '3': {
                const books = manager.filterByRead(true);
                if (books.length === 0) console.log('Нет прочитанных книг.');
                else books.forEach(printBook);
                break;
            }
            case '4': {
                const books = manager.filterByRead(false);
                if (books.length === 0) console.log('Нет непрочитанных книг.');
                else books.forEach(printBook);
                break;
            }
            case '5': {
                const query = await question('Введите часть названия или автора: ');
                const results = manager.searchBooks(query);
                if (results.length === 0) console.log('Книги не найдены.');
                else results.forEach(printBook);
                break;
            }
            case '6': {
                const id = parseInt(await question('Введите ID книги для редактирования: '));
                const book = manager.findBook(id);
                if (!book) { console.log('Книга не найдена.'); continue; }
                console.log('Оставьте поле пустым, чтобы не менять.');
                const newTitle = await question(`Название (${book.title}): `);
                const newAuthor = await question(`Автор (${book.author}): `);
                const newYear = await question(`Год (${book.year}): `);
                const newGenre = await question(`Жанр (${book.genre}): `);
                const newRead = await question(`Статус (1-прочитана, 0-не прочитана) сейчас: ${book.read ? '1' : '0'}: `);
                const newRating = await question(`Оценка (${book.rating}): `);
                const updates = {};
                if (newTitle.trim()) updates.title = newTitle;
                if (newAuthor.trim()) updates.author = newAuthor;
                if (newYear.trim()) {
                    const y = parseInt(newYear);
                    if (!isNaN(y)) updates.year = y;
                    else console.log('Год должен быть числом, пропускаем.');
                }
                if (newGenre.trim()) updates.genre = newGenre;
                if (newRead.trim()) updates.read = newRead === '1';
                if (newRating.trim()) {
                    const r = parseInt(newRating);
                    if (!isNaN(r)) updates.rating = r;
                    else console.log('Оценка должна быть числом, пропускаем.');
                }
                if (manager.editBook(id, updates)) console.log('Книга обновлена.');
                else console.log('Ошибка обновления.');
                break;
            }
            case '7': {
                const id = parseInt(await question('Введите ID книги для удаления: '));
                if (manager.deleteBook(id)) console.log('Книга удалена.');
                else console.log('Книга не найдена.');
                break;
            }
            case '8': {
                const stats = manager.getStats();
                console.log('\n=== СТАТИСТИКА ===');
                console.log(`Всего книг: ${stats.total}`);
                console.log(`Прочитано: ${stats.read}`);
                console.log(`Не прочитано: ${stats.unread}`);
                console.log(`Средняя оценка: ${stats.avgRating.toFixed(2)}`);
                console.log('По жанрам:');
                for (const [genre, count] of Object.entries(stats.genres)) {
                    console.log(`  ${genre}: ${count}`);
                }
                break;
            }
            case '9':
                try {
                    await manager.saveToFile();
                    console.log('Сохранено.');
                } catch (err) {
                    console.log('Ошибка сохранения:', err.message);
                }
                break;
            case '10':
                try {
                    await manager.loadFromFile();
                    console.log('Загружено.');
                } catch (err) {
                    console.log('Ошибка загрузки:', err.message);
                }
                break;
            default:
                console.log('Неизвестная команда.');
        }
    }
    rl.close();
}

main().catch(console.error);
