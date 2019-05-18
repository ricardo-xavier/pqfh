unsigned short getshort(unsigned char *s) {
    return (unsigned short) (s[1] + (s[0] << 8));
}

unsigned int getint(unsigned char *s) {
    return (unsigned int) (s[3] + (s[2] << 8) + (s[1] << 16) + (s[0] << 24));
}

void putshort(unsigned char *s, unsigned short n) {
    s[0] = (unsigned char) (n >> 8);
    s[1] = (unsigned char) (n);
}

void putint(unsigned char *s, unsigned int n) {
    s[0] = (unsigned char) (n >> 24);
    s[1] = (unsigned char) (n >> 16);
    s[2] = (unsigned char) (n >> 8);
    s[3] = (unsigned char) (n);
}
