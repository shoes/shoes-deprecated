# encoding: utf-8
require 'sqlite3'
require 'fileutils'
include FileUtils
tmpdir = File.join(LIB_DIR,"sample-data")
#$stderr.puts "tmpdir #{tmpdir}"
mkdir_p tmpdir
dbfile = File.join(tmpdir, "simple-sqlite3.db")
if File.exists? dbfile
  # this has a bad habit of failing on Windows
  rm dbfile
end
Shoes.app :width => 350, :height => 130 do
  info dbfile
  db = SQLite3::Database.new dbfile
  db.execute "create table t1 (t1key INTEGER PRIMARY KEY,data " \
    "TEXT,num double,timeEnter DATE)"
  db.execute "insert into t1 (data,num) values ('This is sample data',3)"
  db.execute "insert into t1 (data,num) values ('More sample data',6)"
  db.execute "insert into t1 (data,num) values ('Aurélio, Küng, Stärk, Uña, Łuksza',6)"
  #db.execute "insert into t1 (data,num) values ('Aurlio, Kng, Strk, Ua, uksza',6)"
  db.execute "insert into t1 (data,num) values ('And a little more',9)"
  rows = db.execute "select * from t1"
  rows.each{|k, d, n| para "#{k} : #{d} : #{n}\n"}
end

