require 'sqlite3'

data_file = File.join(LIB_DIR, "+data")
data_init = !File.exists?(data_file)

DATABASE = SQLite3::Database.new data_file
DATABASE.type_translation = true

class << DATABASE
  DATABASE_VERSION = 1
  def setup
    DATABASE.execute_batch %{
      CREATE TABLE cache (
        url   text primary key,
        etag  text,
        hash  varchar(40),
        saved datetime
      );
      CREATE TABLE upgrades (
        version int primary key
      );
      INSERT INTO upgrades VALUES (?);
    }, DATABASE_VERSION
  end
  def check_cache_for url
    etag, hash, saved = 
      DATABASE.get_first_row("SELECT etag, hash, saved FROM cache WHERE url = ?", url)
    {:etag => etag, :hash => hash, :saved => saved.nil? ? 0 : Time.parse(saved.to_s).to_i}
  end
  def notify_cache_of url, etag, hash
    DATABASE.query %{
      REPLACE INTO cache (url, etag, hash, saved)
      VALUES (?, ?, ?, datetime("now", "localtime"));
    }, url, etag, hash
    nil
  end
end

DATABASE.setup if data_init
