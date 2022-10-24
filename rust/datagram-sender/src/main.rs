use std::env;
use std::net::{Ipv6Addr, UdpSocket};

const PORT: u16 = 3490;

fn main() {
    let (host, msg) = match (env::args().nth(1), env::args().nth(2)) {
        (Some(h), Some(m)) => (h, m),
        _ => {
            eprintln!("usage: sender hostname message");
            return;
        }
    };

    match UdpSocket::bind((Ipv6Addr::UNSPECIFIED, 0)) {
        Ok(socket) => match socket.send_to(msg.as_bytes(), (host.as_str(), PORT)) {
            Ok(bytes) => println!("sent {bytes} bytes to {host}"),
            Err(e) => eprintln!("failed to write to socket; cause: {e}"),
        },
        Err(e) => eprintln!("failed to bind to socket; cause: {e}"),
    }
}
