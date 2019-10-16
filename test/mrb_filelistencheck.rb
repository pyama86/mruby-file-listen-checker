##
## FileListenCheck Test
##

assert("FileListenCheck#listen?") do
  v4 = Process.fork do
    system "nc -l -4 -p 11111"
  end
  sleep 1

  assert_true(FileListenCheck.new("0.0.0.0", 11111).listen?)
  assert_false(FileListenCheck.new("0.0.0.0", 11112).listen?)
  Process.kill :TERM, v4
end

assert("FileListenCheck#listen6?") do
  v6 = Process.fork do
    system "nc -l -6 -p 11112"
  end
  sleep 1

  assert_false(FileListenCheck.new("::", 11111).listen6?)
  assert_true(FileListenCheck.new("::", 11112).listen6?)
  Process.kill :TERM, v6
end


assert("change ns") do
  pid1 = Process.fork do
    Namespace.unshare(Namespace::CLONE_NEWNS)
    Namespace.unshare(Namespace::CLONE_NEWNET)
    system "nc -l -4 -p 11113"
  end
  sleep 1
  ns = Namespace.nsenter(Namespace::CLONE_NEWNS|Namespace::CLONE_NEWNET, :pid => pid1) {
    exit 0 if FileListenCheck.new("0.0.0.0", 11113).listen?
    exit 1
  } rescue nil

  assert_true ns.success?
  Process.kill :TERM, pid1
end
