require 'ceedling/plugin'

class Flash < Plugin

  def setup
    # Register a rake task named :flash
    task :flash do
      sh 'openocd -f ./openocd.cfg -c "program /workspace/build/debug/HVC.elf verify reset exit"'
    end
  end

end