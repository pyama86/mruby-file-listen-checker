##
## FileListenCheck Test
##

assert("FileListenCheck#listen?") do
  v4 = Process.fork do
    system "nc -l -4 -p 11111"
  end
  sleep 2

  assert_true(FileListenCheck.new("0.0.0.0", 11111).listen?)
  assert_false(FileListenCheck.new("0.0.0.0", 11112).listen?)
  Process.kill :TERM, v4
end


assert("FileListenCheck#listen6?") do
  v6 = Process.fork do
    system "nc -l -6 -p 11112"
  end
  sleep 2

  puts "="*30
  puts `netstat -an |grep -i listen`
  puts "="*30
  assert_false(FileListenCheck.new("::", 11111).listen6?)
  assert_true(FileListenCheck.new("::", 11112).listen6?)
  Process.kill :TERM, v6
end if File.exists?("/proc/net/tcp6")
