module DiyProf
  class Tracer
    include TimeHelpers

    def initialize(reporter)
      @reporter = reporter
      @tracepoints = [:call, :return].collect do |event|
        TracePoint.new(event) do |trace|
          reporter.record(event, trace.method_id, cpu_time)
        end
      end
    end
    def enable
      @tracepoints.each(&:enable)
    end

    def disable
      @tracepoints.each(&:disable)
    end

    def result
      @reporter.result
    end
  end
end
