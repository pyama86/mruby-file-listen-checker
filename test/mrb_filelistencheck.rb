##
## FileListenCheck Test
##

assert("FileListenCheck#listen?") do
  v4 = Process.fork do
    system "nc -4 -l 0.0.0.0 11111"
  end
  sleep 1
  assert_true(FileListenCheck.new("0.0.0.0", 11111).listen?, "listen ok")
  assert_false(FileListenCheck.new("0.0.0.0", 11112).listen?, "does not listen")
  Process.kill :TERM, v4
end


assert("FileListenCheck#listen6?") do
  v6 = Process.fork do
    system "nc -6 -l :: 11112"
  end
  sleep 1
  assert_false(FileListenCheck.new("::", 11111).listen6?, "does not listen")
  assert_true(FileListenCheck.new("::", 11112).listen6?, "listen ok")
  Process.kill :TERM, v6
end if File.exists?("/proc/net/tcp6")
