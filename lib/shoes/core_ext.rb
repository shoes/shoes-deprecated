# Shoes-specific extensions to core Ruby classes
class Encoding
 %w[ASCII_8BIT UTF_16BE UTF_16LE UTF_32BE UTF_32LE US_ASCII].each do |ec|
   eval "#{ec} = '#{ec.sub '_', '-'}'"
 end unless RUBY_PLATFORM =~ /linux/ or RUBY_PLATFORM =~ /darwin/
end

class Range
  def rand
    conv = (Integer === self.end && Integer === self.begin ? :to_i : :to_f)
    ((Kernel.rand * (self.end - self.begin)) + self.begin).send(conv)
  end
end

unless Time.respond_to? :today
  def Time.today
    t = Time.now
    t - (t.to_i % 86400)
  end
end
