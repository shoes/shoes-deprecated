module DiyProf::TimeHelpers
  # These methods make use of `clock_gettime` method introduced in Ruby 2.1
  # to measure CPU time and Wall clock time.

  def cpu_time
    Process.clock_gettime(Process::CLOCK_PROCESS_CPUTIME_ID, :microsecond)
  end

  def wall_time
    Process.clock_gettime(Process::CLOCK_MONOTONIC, :microsecond)
  end
end
