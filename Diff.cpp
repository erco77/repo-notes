    //DEBUG cout << "Loading diffs from hash " << hash << ": ";
    //DEBUG cout << lines.size() << " loaded." << endl;
    int diff_line_num = 0;
        //DEBUG cout << "Working on: " << s << endl;
            //DEBUG cout << "--- DIFF FILENAME: " << diff_filename << endl;
            diff_line_num = 0;      // filename becomes line #0
        diff.add_line(lines[i], ++diff_line_num);   // one based