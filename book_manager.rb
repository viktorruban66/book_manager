# book_manager.rb
require 'json'
require 'date'

class Book
  attr_accessor :id, :title, :author, :year, :genre, :read, :rating, :added_date

  def initialize(id, title, author, year, genre, read, rating, added_date = Date.today.to_s)
    @id = id
    @title = title
    @author = author
    @year = year
    @genre = genre
    @read = read
    @rating = rating
    @added_date = added_date
  end

  def to_h
    { id: @id, title: @title, author: @author, year: @year, genre: @genre,
      read: @read, rating: @rating, added_date: @added_date }
  end

  def self.from_h(hash)
    Book.new(hash[:id], hash[:title], hash[:author], hash[:year],
             hash[:genre], hash[:read], hash[:rating], hash[:added_date])
  end
end

class BookManager
  attr_reader :books

  def initialize
    @books = []
    @next_id = 1
  end

  def add_book(title, author, year, genre, read, rating)
    raise "Оценка должна быть от 1 до 10" unless (1..10).include?(rating)
    raise "Год должен быть от 0 до #{Date.today.year}" unless (0..Date.today.year).include?(year)
    book = Book.new(@next_id, title, author, year, genre, read, rating)
    @books << book
    @next_id += 1
    book
  end

  def find_book(id)
    @books.find { |b| b.id == id }
  end

  def edit_book(id, **kwargs)
    book = find_book(id)
    return false unless book
    kwargs.each do |key, value|
      book.send("#{key}=", value) if book.respond_to?("#{key}=")
    end
    true
  end

  def delete_book(id)
    book = find_book(id)
    return false unless book
    @books.delete(book)
    true
  end

  def search_books(query)
    q = query.downcase
    @books.select { |b| b.title.downcase.include?(q) || b.author.downcase.include?(q) }
  end

  def filter_by_read(read)
    @books.select { |b| b.read == read }
  end

  def filter_by_genre(genre)
    @books.select { |b| b.genre.downcase == genre.downcase }
  end

  def stats
    total = @books.size
    read_count = filter_by_read(true).size
    unread = total - read_count
    ratings = filter_by_read(true).map(&:rating)
    avg_rating = ratings.empty? ? 0 : ratings.sum.to_f / ratings.size
    genres = Hash.new(0)
    @books.each { |b| genres[b.genre] += 1 }
    { total: total, read: read_count, unread: unread, avg_rating: avg_rating, genres: genres }
  end

  def save_to_file(filename = "books_data.json")
    data = { books: @books.map(&:to_h) }
    File.write(filename, JSON.pretty_generate(data))
  end

  def load_from_file(filename = "books_data.json")
    return unless File.exist?(filename)
    data = JSON.parse(File.read(filename), symbolize_names: true)
    @books.clear
    data[:books].each do |item|
      book = Book.from_h(item)
      @books << book
      @next_id = book.id + 1 if book.id >= @next_id
    end
  rescue JSON::ParserError
    puts "Ошибка чтения файла."
  end
end

def print_book(book)
  status = book.read ? "✅ Прочитана" : "📖 Не прочитана"
  puts "##{book.id} - #{book.title} (#{book.year})"
  puts "   Автор: #{book.author}, Жанр: #{book.genre}"
  puts "   #{status}, Оценка: #{book.rating}/10"
  puts "   Добавлена: #{book.added_date}"
end

def main
  manager = BookManager.new
  manager.load_from_file

  loop do
    puts "\n===== МЕНЕДЖЕР КНИГ (Ruby) ====="
    puts "1. Добавить книгу"
    puts "2. Показать все книги"
    puts "3. Показать прочитанные книги"
    puts "4. Показать непрочитанные книги"
    puts "5. Найти книги по автору или названию"
    puts "6. Редактировать книгу"
    puts "7. Удалить книгу"
    puts "8. Показать статистику"
    puts "9. Сохранить в файл"
    puts "10. Загрузить из файла"
    puts "0. Выход"
    print "Выберите действие: "
    choice = gets.chomp

    case choice
    when "0"
      break
    when "1"
      print "Название: "
      title = gets.chomp
      next if title.empty?
      print "Автор: "
      author = gets.chomp
      next if author.empty?
      print "Год издания: "
      year = gets.chomp.to_i
      print "Жанр: "
      genre = gets.chomp
      genre = "Неизвестно" if genre.empty?
      print "Статус (1-прочитана, 0-не прочитана): "
      read = gets.chomp == "1"
      print "Оценка (1-10): "
      rating = gets.chomp.to_i
      begin
        book = manager.add_book(title, author, year, genre, read, rating)
        puts "Книга добавлена с ID #{book.id}"
      rescue => e
        puts "Ошибка: #{e.message}"
      end
    when "2"
      if manager.books.empty?
        puts "Нет книг."
      else
        manager.books.each { |b| print_book(b) }
      end
    when "3"
      books = manager.filter_by_read(true)
      if books.empty?
        puts "Нет прочитанных книг."
      else
        books.each { |b| print_book(b) }
      end
    when "4"
      books = manager.filter_by_read(false)
      if books.empty?
        puts "Нет непрочитанных книг."
      else
        books.each { |b| print_book(b) }
      end
    when "5"
      print "Введите часть названия или автора: "
      query = gets.chomp
      results = manager.search_books(query)
      if results.empty?
        puts "Книги не найдены."
      else
        results.each { |b| print_book(b) }
      end
    when "6"
      print "Введите ID книги для редактирования: "
      id = gets.chomp.to_i
      book = manager.find_book(id)
      unless book
        puts "Книга не найдена."
        next
      end
      puts "Оставьте поле пустым, чтобы не менять."
      print "Название (#{book.title}): "
      new_title = gets.chomp
      print "Автор (#{book.author}): "
      new_author = gets.chomp
      print "Год (#{book.year}): "
      new_year = gets.chomp
      print "Жанр (#{book.genre}): "
      new_genre = gets.chomp
      print "Статус (1-прочитана, 0-не прочитана) сейчас: #{book.read ? '1' : '0'}: "
      new_read = gets.chomp
      print "Оценка (#{book.rating}): "
      new_rating = gets.chomp
      updates = {}
      updates[:title] = new_title unless new_title.empty?
      updates[:author] = new_author unless new_author.empty?
      unless new_year.empty?
        updates[:year] = new_year.to_i
      end
      updates[:genre] = new_genre unless new_genre.empty?
      unless new_read.empty?
        updates[:read] = new_read == "1"
      end
      unless new_rating.empty?
        updates[:rating] = new_rating.to_i
      end
      if manager.edit_book(id, **updates)
        puts "Книга обновлена."
      else
        puts "Ошибка обновления."
      end
    when "7"
      print "Введите ID книги для удаления: "
      id = gets.chomp.to_i
      if manager.delete_book(id)
        puts "Книга удалена."
      else
        puts "Книга не найдена."
      end
    when "8"
      stats = manager.stats
      puts "\n=== СТАТИСТИКА ==="
      puts "Всего книг: #{stats[:total]}"
      puts "Прочитано: #{stats[:read]}"
      puts "Не прочитано: #{stats[:unread]}"
      puts "Средняя оценка: #{'%.2f' % stats[:avg_rating]}"
      puts "По жанрам:"
      stats[:genres].each { |g, c| puts "  #{g}: #{c}" }
    when "9"
      manager.save_to_file
      puts "Сохранено."
    when "10"
      manager.load_from_file
      puts "Загружено."
    else
      puts "Неизвестная команда."
    end
  end
end

main if __FILE__ == $0
