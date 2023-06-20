@rem requires you to have already created the cut out .getc and .getp objects
d:\tools\bin2inc newtitle.tiac ..\ssa\title_c.c TITLEC
d:\tools\bin2inc newtitle.tiap ..\ssa\title_p.c TITLEP
d:\tools\bin2inc ship1.getc ship1_c.c SHIP1C
d:\tools\bin2inc ship1.getp ship1_p.c SHIP1P
d:\tools\bin2inc ship2.getc ship2_c.c SHIP2C
d:\tools\bin2inc ship2.getp ship2_p.c SHIP2P
d:\tools\bin2inc ship3.getc ship3_c.c SHIP3C
d:\tools\bin2inc ship3.getp ship3_p.c SHIP3P
d:\tools\bin2inc ship4.getc ship4_c.c SHIP4C
d:\tools\bin2inc ship4.getp ship4_p.c SHIP4P

goto :EOF

    // extract just the spaceship from the SSA title page
    {
        int startoff = 7*32*8;//+16*8;
        int n,p;

        FILE *fp = fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP1.TIAc", "rb");
        fread(buffer, 1, 6144, fp);
        fclose(fp);
        // we need to extract 16x17 characters from the bottom right
        // so that starts at 16,7
        fp=fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP1.getc", "wb");
        p = startoff;
        for (int y=0; y<17; ++y) {
            fwrite(&buffer[p], 1, 16*8, fp);
            p+=32*8;
        }
        fflush(fp);
        n = ftell(fp);
        fclose(fp);

        fp = fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP1.TIAp", "rb");
        fread(buffer, 1, 6144, fp);
        fclose(fp);
        // we need to extract 16x17 characters from the bottom right
        // so that starts at 16,7
        fp=fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP1.getp", "wb");
        p = startoff;
        for (int y=0; y<17; ++y) {
            fwrite(&buffer[p], 1, 16*8, fp);
            p+=32*8;
        }
        fflush(fp);
        n = ftell(fp);
        fclose(fp);

        fp = fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP2.TIAp", "rb");
        fread(buffer, 1, 6144, fp);
        fclose(fp);
        // we need to extract 16x17 characters from the bottom right
        // so that starts at 16,7
        fp=fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP2.getp", "wb");
        p = startoff;
        for (int y=0; y<17; ++y) {
            fwrite(&buffer[p], 1, 16*8, fp);
            p+=32*8;
        }
        fflush(fp);
        n = ftell(fp);
        fclose(fp);

        fp = fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP2.TIAc", "rb");
        fread(buffer, 1, 6144, fp);
        fclose(fp);
        // we need to extract 16x17 characters from the bottom right
        // so that starts at 16,7
        fp=fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP2.getc", "wb");
        p = startoff;
        for (int y=0; y<17; ++y) {
            fwrite(&buffer[p], 1, 16*8, fp);
            p+=32*8;
        }
        fflush(fp);
        n = ftell(fp);
        fclose(fp);

        fp = fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP3.TIAp", "rb");
        fread(buffer, 1, 6144, fp);
        fclose(fp);
        // we need to extract 16x17 characters from the bottom right
        // so that starts at 16,7
        fp=fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP3.getp", "wb");
        p = startoff;
        for (int y=0; y<17; ++y) {
            fwrite(&buffer[p], 1, 16*8, fp);
            p+=32*8;
        }
        fflush(fp);
        n = ftell(fp);
        fclose(fp);

        fp = fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP3.TIAc", "rb");
        fread(buffer, 1, 6144, fp);
        fclose(fp);
        // we need to extract 16x17 characters from the bottom right
        // so that starts at 16,7
        fp=fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP3.getc", "wb");
        p = startoff;
        for (int y=0; y<17; ++y) {
            fwrite(&buffer[p], 1, 16*8, fp);
            p+=32*8;
        }
        fflush(fp);
        n = ftell(fp);
        fclose(fp);

        fp = fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP4.TIAp", "rb");
        fread(buffer, 1, 6144, fp);
        fclose(fp);
        // we need to extract 16x17 characters from the bottom right
        // so that starts at 16,7
        fp=fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP4.getp", "wb");
        p = startoff;
        for (int y=0; y<17; ++y) {
            fwrite(&buffer[p], 1, 16*8, fp);
            p+=32*8;
        }
        fflush(fp);
        n = ftell(fp);
        fclose(fp);

        fp = fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP4.TIAc", "rb");
        fread(buffer, 1, 6144, fp);
        fclose(fp);
        // we need to extract 16x17 characters from the bottom right
        // so that starts at 16,7
        fp=fopen("D:\\work\\SuperSpaceAcer\\coleco\\SuperSpaceAcer\\title2\\SHIP4.getc", "wb");
        p = startoff;
        for (int y=0; y<17; ++y) {
            fwrite(&buffer[p], 1, 16*8, fp);
            p+=32*8;
        }
        fflush(fp);
        n = ftell(fp);
        fclose(fp);
    }


