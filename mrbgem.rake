MRuby::Gem::Specification.new('mruby-file-listen-checker') do |spec|
  spec.license = 'MIT'
  spec.authors = 'pyama86'
  spec.add_test_dependency 'mruby-process'
  spec.add_test_dependency 'mruby-linux-namespace', git: 'https://github.com/haconiwa/mruby-linux-namespace'
end
