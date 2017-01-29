# --                                                            ; {{{1
#
# File        : open_uri_w_redirect_to_https.rb
# Maintainer  : Felix C. Stegerman <flx@obfusk.net>
# Date        : 2014-11-26
#
# Copyright   : Copyright (C) 2014  Felix C. Stegerman
# Licence     : MIT
#
# --                                                            ; }}}1

require 'open-uri'
require 'thread'

module OpenURI

  RedirectHTTPToHTTPS = { mutex: Mutex.new, default: false }

  class << self
    alias_method :open_uri_orig     , :open_uri
    alias_method :redirectable_orig?, :redirectable?

    # set the `open_uri` `:redirect_to_https` global default
    def redirect_to_https=(val)
      (x = RedirectHTTPToHTTPS)[:mutex].synchronize { x[:default] = val }
    end

    # get the `open_uri` `:redirect_to_https` dynamic or global
    # default
    def redirect_to_https?
      (x = RedirectHTTPToHTTPS)[:mutex].synchronize do
        t = Thread.current[:__open_uri_w_redirect_to_https_default__]
        t.nil? ? x[:default] : t
      end
    end

    # dynamically thread-scoped `open_uri` `:redirect_to_https`
    # default; for example:
    #
    # ```
    # w_redirect_to_https { open('http://github.com' }
    # ```
    def w_redirect_to_https(val = true, &b)
      old = Thread.current[:__open_uri_w_redirect_to_https_default__]
      Thread.current[:__open_uri_w_redirect_to_https_default__] = val
      begin
        b[]
      ensure
        Thread.current[:__open_uri_w_redirect_to_https_default__] = old
      end
    end

    # `redirectable?` patch that uses a thread-local variable to
    # determine whether HTTP to HTTPS redirection should be allowed
    # (as well)
    #
    # unless the dynamic or global `:redirect_to_https` setting is set
    # to `:always`, only the behaviour of calls through `open_uri`
    # will be changed (as per argument or dynamic or global setting)
    def redirectable?(uri1, uri2)
      if  redirect_to_https? == :always ||
          Thread.current[:__open_uri_w_redirect_to_https__]
        redirectable_w_redirect_to_https? uri1, uri2
      else
        redirectable_orig? uri1, uri2
      end
    end

    # `open_uri` patch that also accepts the `:redirect_to_https`
    # option; when set to `true`, redirections from HTTP to HTTPS are
    # allowed; for example:
    #
    # ```
    # open('http://github.com', redirect_to_https: true)
    # ```
    #
    # you can set the dynamic or global default using
    # `redirect_to_https=` or `w_redirect_to_https`
    def open_uri(name, *rest, &b)
      r = (o = rest.find { |x| Hash === x }) && o.delete(:redirect_to_https)
      Thread.current[:__open_uri_w_redirect_to_https__] = \
        r.nil? ? redirect_to_https? : r
      b2 = -> io {
        Thread.current[:__open_uri_w_redirect_to_https__] = nil; b[io]
      }
      begin
        open_uri_orig name, *rest, &(b ? b2 : nil)
      ensure
        Thread.current[:__open_uri_w_redirect_to_https__] = nil
      end
    end

  private

    # allow everything the `redirectable?` method does as well as HTTP
    # to HTTPS
    def redirectable_w_redirect_to_https?(uri1, uri2)
      redirectable_orig?(uri1, uri2) || \
        (uri1.scheme.downcase == 'http' && uri2.scheme.downcase == 'https')
    end

  end
end

