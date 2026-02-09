require 'ceedling/plugin'

class Crelease < Plugin

  def setup
    # Register a rake task named :flash
    task :crelease do
      sh 'ceedling release'
      sh 'mkdir /workspace/build/debug'
      sh 'cp /tmp/hvc-build/release/HVC.elf /workspace/build/debug/HVC.elf'
    end
  end

end