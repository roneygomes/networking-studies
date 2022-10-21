use std::{env, io};
use std::net::SocketAddr;

use dns_lookup as dns;

fn main() {
    let hostname = env::args().nth(1);

    if hostname.is_none() {
        eprintln!("usage: showip hostname");
        return;
    }

    let hints = dns::AddrInfoHints {
        socktype: dns::SockType::Stream.into(),
        ..dns::AddrInfoHints::default()
    };

    let result = dns::getaddrinfo(hostname.as_deref(), None, Some(hints));

    match result {
        Ok(addresses) => for address in addresses {
            match address {
                Ok(addr) => {
                    match addr.sockaddr {
                        SocketAddr::V4(a) => println!("  IPv4: {a}"),
                        SocketAddr::V6(a) => println!("  IPv6: {a}")
                    }
                }
                Err(e) => eprintln!("{e}")
            }
        }
        Err(e) => {
            let io_err: io::Error = e.into();
            eprintln!("getaddrinfo error: {io_err}");
        },
    }
}
