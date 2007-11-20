module Kernel
    def inspect(hits = {})
        return "(#{self.class} ...)" if hits[self]
        hits[self] = true
        if instance_variables.empty?
            "(#{self.class})"
        else
            "(#{self.class} " + 
                instance_variables.map do |x|
                    v = instance_variable_get(x)
                    "#{x}=" + (v.method(:inspect).arity == 0 ? v.inspect : v.inspect(hits))
                end.join(' ') +
            ")"
        end
      end
    #def to_html
    # obj = self
    #  Web.Bit {
    #    h1 "A #{obj.class}"
    #  }
    #end
    def to_s; inspect end
end

class Array
    def inspect(hits = {})
        return "[...]" if hits[self]
        hits[self] = true
        "[" + map { |x| x.inspect(hits) }.join(', ') + "]"
    end
    def to_html
      ary = self
      Web.Bit {
        h5 "A List of Things"
        h1 "An Array"
        unless ary.empty?
          ol {
            ary.map { |x| li { self << HTML(x) } }
          }
        end
      }
    end
    def / len
      a = []
      each_with_index do |x, i|
        a << [] if i % len == 0
        a.last << x
      end
      a
    end
end

class Hash
    def inspect(hits = {})
        return "{...}" if hits[self]
        hits[self] = true
        "{" + map { |k,v| k.inspect(hits) + "=>" + v.inspect(hits) }.join(', ') + "}"
    end
    def to_html
      h = self
      Web.Bit {
        h5 "Pairs of Things"
        h1 "A Hash"
        unless h.empty?
          ul {
            h.each { |k, v| li { self << "<div class='hashkey'>#{HTML(k)}</div><div class='hashvalue'>#{HTML(v)}</div>" } }
          }
        end
      }
    end
end

class File
    def inspect(hits = nil)
        "(#{self.class} #{path})"
    end
end

class Proc
    def inspect(hits = nil)
        v = "a"
        pvars = []
        (arity < 0 ? -(arity+1) : arity).times do |i|
            pvars << v
            v = v.succ
        end
        pvars << "*#{v}" if arity < 0
        "(Proc |#{pvars.join(',')}|)"
    end
end

class Class
    def make_inspect m = :inspect
        alias_method :the_original_inspect, m
        class_eval %{
            def inspect(hits = nil)
                the_original_inspect
            end
    }
      #def to_html(hits = nil)
      #  #{m} + " <div class='classname'>" + self.class.name + "</div>"
      #end
    end
end

class Module; make_inspect :name end
class Regexp; make_inspect end
class String; make_inspect end
class Symbol; make_inspect end
class Time; make_inspect end
class Numeric; make_inspect :to_s end
class Bignum; make_inspect :to_s end
class Fixnum; make_inspect :to_s end
class Float; make_inspect :to_s end
class TrueClass; make_inspect :to_s end
class FalseClass; make_inspect :to_s end
class NilClass; make_inspect end
