use std::net::{Ipv6Addr, SocketAddr, UdpSocket};

const BUFFER_LENGTH: usize = 128;
const PORT: u16 = 3490;

fn main() {
    match UdpSocket::bind((Ipv6Addr::LOCALHOST, PORT)) {
        Ok(socket) => {
            let mut buf = [0; BUFFER_LENGTH];

            match socket.recv_from(&mut buf) {
                Ok((bytes, addr)) => {
                    display_packet_content(&buf, bytes, addr);
                },
                Err(e) => eprintln!("failed to read from socket; cause: {e}"),
            }
        }
        Err(e) => eprintln!("failed to bind to socket; cause: {e}"),
    }
}

fn display_packet_content(buf: &[u8], buf_len: usize, addr: SocketAddr) {
    println!("received {buf_len} bytes from {addr}");

    if let Ok(s) = String::from_utf8(buf.to_vec()) {
        println!("packet contains {s}");
    } else {
        eprintln!("can't decoded packet as utf-8 string");
    }
}
