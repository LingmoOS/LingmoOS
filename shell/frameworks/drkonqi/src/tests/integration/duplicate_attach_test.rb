# SPDX-FileCopyrightText: 2017 Harald Sitter <sitter@kde.org>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

require_relative 'test_helper'

require 'json'
require 'webrick'

# monkey patch alias from put to get, upstream only aliases post -.-
class WEBrick::HTTPServlet::ProcHandler
  alias do_PUT do_GET
end

class TestDuplicateAttach < ATSPITest
  def setup
    server = WEBrick::HTTPServer.new(Port: 0)

    port = server.config.fetch(:Port)
    ENV['DRKONQI_KDE_BUGZILLA_URL'] = "http://localhost:#{port}/"

    @got_comment = false

    server.mount_proc '/' do |req, res|
      query = req.request_uri.query

      case req.request_method
      when 'GET'
        case req.path_info
        when '/rest/version'
          res.body = JSON.generate(version: '5.0.6')
          next
        when '/rest/product/ruby' # this is off because the product detection is a bit meh
          res.body = File.read("#{__dir__}/data/product.dolphin")
          next
        when '/rest/login'
          unless query.include?('login=xxx') && query.include?('password=yyy')
            abort
          end

          res.body = JSON.generate(token: '123', id: '321')
          next
        when '/rest/bug', '/rest/bug/375161'
          res.body = File.read("#{__dir__}/data/bugs")
          next
        when '/rest/bug/375161/comment'
          res.body = File.read("#{__dir__}/data/comments")
          next
        end
      when 'PUT'
        case req.path_info
        when '/rest/bug/375161'
          input = JSON.parse(req.body)
          if input['cc']&.[]('add')&.include?('xxx')
            res.body = File.read("#{__dir__}/data/bugs")
            next
          end
        end
      when 'POST'
        case req.path_info
        when '/rest/bug/375161/attachment'
          input = JSON.parse(req.body)
          if input['comment']&.include?('yyyyyyyyy')
            res.body = JSON.generate(ids: [375161])
            @got_comment = true
            next
          end
        end
      end

      warn "!!!!!!!!"
      res.keep_alive = false
      abort "ERROR Unexpected request #{req}"
    end

    Thread.report_on_exception = true
    @api_server_thread = Thread.start { server.start }
    @api_server_thread.report_on_exception=true

    @tracee = fork { loop { sleep(999_999_999) } }

    assert File.exist?(DRKONQI_PATH), "drkonqi not at #{DRKONQI_PATH}"
    @drkonqi_pid = spawn(DRKONQI_PATH,
                         '--signal', '11',
                         '--pid', @tracee.to_s,
                         '--bugaddress', 'submit@bugs.kde.org',
                         '--dialog')
    puts "pid: #{@drkonqi_pid}"
  end

  def teardown
    Process.kill('KILL', @tracee)
    Process.waitpid2(@tracee)
    @api_server_thread.kill
    @api_server_thread.join
  end

  def drkonqi_running?
    Process.waitpid(@drkonqi_pid, Process::WNOHANG).nil?
  end

  # When evaluating duplicates
  def test_duplicate_attach
    drkonqi = nil

    8.times do # be gracious for drkonqi to come up an atspi
      drkonqi = ATSPI.applications.find { |x| x.name == 'drkonqi' }
      break if drkonqi
      sleep 2
    end

    refute_nil drkonqi, 'Could not find drkonqi on atspi api.' \
                        " It is running: #{drkonqi_running?}"

    accessible = find_in(drkonqi.windows[-1], name: 'Report Bug')
    press(accessible)

    find_in(drkonqi, name: 'Crash Reporting Assistant') do |window|
      accessible = find_in(window, name: 'Next')
      press(accessible)

      accessible = find_in(window, name: 'Yes')
      toggle_on(accessible)

      accessible = find_in(window, name: /^What I was doing when the application.+/)
      toggle_on(accessible)

      accessible = find_in(window, name: 'Next')
      press(accessible)

      loop do
        # Drkonqi is now doing the trace, wait until it is done.
        accessible = find_in(window, name: 'Next')
        refute_nil accessible
        if accessible.states.include?(:sensitive)
          press(accessible)
          break
        end
        warn accessible.states
        sleep 2
      end

      # Set pseudo login data if there are none.
      accessible = find_in(window, name: 'Username input')
      accessible.text.set_to 'xxx' if accessible.text.length <= 0
      # the lineedit is in fact an element on the input. why wouldn't it be...
      accessible = find_in(window, name: 'Password input').children[0]
      accessible.text.set_to 'yyy' if accessible.text.length <= 0

      accessible = find_in(window, name: 'Login')
      press(accessible)

      sleep 2 # Wait for login and bug listing

      accessible = find_in(window, name: '375161')
      toggle_on(accessible)

      accessible = find_in(window, name: 'Open selected report')
      press(accessible)
    end

    find_in(drkonqi, name: 'Bug Description') do |window|
      accessible = find_in(window, name: 'Suggest this crash is related')
      press(accessible)
    end

    find_in(drkonqi, name: 'Related Bug Report') do |window|
      accessible = find_in(window, name: /^Completely sure: attach my information.+/)
      toggle_on(accessible)

      accessible = find_in(window, name: 'Continue')
      press(accessible)
    end

    find_in(drkonqi, name: 'Crash Reporting Assistant') do |window|
      accessible = find_in(window, name: /^The report is going to be attached.+/)
      refute_nil accessible

      accessible = find_in(window, name: 'Next')
      press(accessible)

      accessible = find_in(window, name: 'Information about the crash text')
      accessible.text.set_to(accessible.text.to_s +
        Array.new(128).collect { 'y' }.join)

      accessible = find_in(window, name: 'Next')
      press(accessible)

      accessible = find_in(window, name: 'Submit')
      press(accessible)

      accessible = find_in(window, name: /.*Crash report sent.*/)
      refute_nil accessible

      accessible = find_in(window, name: 'Finish')
      press(accessible)
    end

    assert @got_comment # only true iff the server go tour yyyyyy garbage string
  end
end
