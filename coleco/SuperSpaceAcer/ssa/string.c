// memset function
void memset(char *p, unsigned char ch, int cnt) {
	while (cnt >= 4) {
		*(p++)=ch;
		*(p++)=ch;
		*(p++)=ch;
		*(p++)=ch;
		cnt -= 4;
	}
	while (cnt--) {
		*(p++)=ch;
	}
}

void memcpy(char *dst, const char *src, int cnt) {
    while (cnt > 4) {
        *(dst++) = *(src++);
        *(dst++) = *(src++);
        *(dst++) = *(src++);
        *(dst++) = *(src++);
        cnt -= 4;
    }
    while (cnt--) {
        *(dst++) = *(src++);
    }
}

// this function might crash... untested
void memmove(char *dst, const char *src, int cnt) {
    // if we are not overlapping, then we can just use memcpy
    if ((dst > src+cnt)||(dst+cnt < src)) {
        return memcpy(dst, src, cnt);
    }

    // figure out if we need to count up or down
    if (src > dst) {
        // can count up, just use memcpy
        return memcpy(dst, src, cnt);
    }

    // have to count down
    dst = dst + cnt - 1;
    src = src + cnt - 1;
    while (cnt > 4) {
        *(dst--) = *(src--);
        *(dst--) = *(src--);
        *(dst--) = *(src--);
        *(dst--) = *(src--);
        cnt -= 4;
    }
    while (cnt--) {
        *(dst--) = *(src--);
    }
}

void strcpy(char *p, const char *s) {
    while (*s) {
        *(p++)=*(s++);
    }
}

int strlen(const char *s) {
    int x = 0;
    while (*(s++)) { ++x; }
    return x;
}
