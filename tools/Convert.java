/*
	JBDoc - Documentation for JBit
	Copyright (C) 2012-2013  Emanuele Fornara
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
	
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.OutputStream;
import java.io.PrintStream;
import java.util.ArrayList;

public class Convert {

	public static final int PARLEN = 78;

	public static String normalizeParagraphContent(String s) {
		s = s.replaceAll("\\r", "");
		s = s.replaceAll("\\n", " ");
		s = s.replaceAll(" +", " ");
		s = s.replaceAll(" +$", "");
		return s;
	}

	public static String refillParagraph(String paragraph, int length) {
		StringBuilder builder = new StringBuilder();
		if (length < 20)
			length = 20;
		int pos = 0;
		while (pos < paragraph.length()) {
			int end = pos + length;
			if (end >= paragraph.length()) {
				builder.append(paragraph.substring(pos));
				builder.append('\n');
				break;
			}
			int i;
			for (i = end - 1; i > pos; i--) {
				if (paragraph.charAt(i) == ' ')
					break;
			}
			if (i == pos) {
				for (i = end; i < paragraph.length(); i++) {
					if (paragraph.charAt(i) == ' ')
						break;
				}
			}
			builder.append(paragraph.substring(pos, i));
			builder.append('\n');
			pos = i + 1;
		}
		return builder.toString();
	}

	public static String normalizePreformattedContent(String s) {
		s = s.replaceAll("\\r", "");
		return s;
	}

	public static String text2html(String s) {
		s = s.replaceAll("&", "&amp;");
		s = s.replaceAll("<", "&lt;");
		s = s.replaceAll(">", "&gt;");
		return s;
	}

	public static interface Processor {

		void setOutputStream(OutputStream out);

		int getNumberOfPasses();

		void begin(int pass) throws Exception;

		void section(char type, String content);

		void end(int pass) throws Exception;
	}

	public static class HTMLProcessor implements Processor {

		private final String htmlStart = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
				+ "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\">\n"
				+ "<head>\n"
				+ "<title>JBDoc - __TITLE__</title>\n"
				+ "<meta http-equiv=\"content-type\" content=\"text/html; charset=iso-8859-1\" />\n"
				+ "<meta name=\"viewport\" content=\"width=device-width\" />\n"
				+ "<meta name=\"HandheldFriendly\" content=\"true\" />\n"
				+ "<style type=\"text/css\">\n"
				+ "body { font-family: sans-serif; }\n"
				+ ".nav { font-size: 70%; }\n"
				+ "#links a { margin: 0em 0.5em; }\n"
				+ "</style>\n"
				+ "</head>\n"
				+ "<body>\n"
				+ "<p align=\"right\"><a class=\"nav\" href=\"#content\">skip nav</a></p>\n"
				+ "";

		private final String htmlEnd = "<p><a class=\"nav\" href=\"#top\">top</a></p>\n"
				+ "</body>\n" + "</html>\n" + "";

		private final String epubStart = "<?xml version='1.0' encoding='UTF-8'?>\n"
				+ "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.1//EN' 'http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd'>\n"
				+ "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
				+ "<head>\n"
				+ "<title>JBDoc - __TITLE__</title>\n"
				+ "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n"
				+ "<link href=\"style.css\" type=\"text/css\" rel=\"stylesheet\" />\n"
				+ "</head>\n" + "<body>\n" + "";

		private final String epubEnd = "</body>\n" + "</html>\n" + "";

		private final boolean epub;
		private PrintStream out;
		private int pass;
		private String title;
		private int id;

		private ArrayList list = new ArrayList();

		public HTMLProcessor(boolean epub) {
			this.epub = epub;
		}

		public void setOutputStream(OutputStream out) {
			this.out = new PrintStream(out);
		}

		public int getNumberOfPasses() {
			return 2;
		}

		private void outputEPubStart() {
			out.print(epubStart.replaceAll("__TITLE__", title));
		}

		private void outputHTMLStart() {
			out.print(htmlStart.replaceAll("__TITLE__", title));
			out.print("<p id=\"top\">\n");
			for (int i = 0; i < list.size(); i++)
				out.print("<a class=\"nav\" href=\"#id" + (i + 1) + "\">"
						+ list.get(i) + "</a><br />\n");
			out.print("</p>\n");
			out.print("<div id=\"content\"></div>\n");
		}

		public void begin(int pass) throws Exception {
			this.pass = pass;
			id = 0;
			if (pass == 0)
				return;
			if (title == null)
				title = "Untitled";
			if (epub)
				outputEPubStart();
			else
				outputHTMLStart();
		}

		private void outputHeader(char type, String content) {
			content = content.replaceAll("\\n", "");
			String el = "h6";
			switch (Character.toLowerCase(type)) {
			case 'a':
				el = "h1";
				break;
			case 'b':
				el = "h2";
				break;
			}
			String idAttr = "";
			if (!epub && Character.isUpperCase(type)) {
				if (id != 0)
					out.print("<p><a class=\"nav\" href=\"#top\">top</a></p>\n");
				out.print("<hr />\n");
				idAttr = " id=\"id" + (++id) + "\"";
			}
			out.print("<" + el + idAttr + ">" + content + "</" + el + ">\n");
		}

		private void analyzeSection(char type, String content) {
			if (title == null && Character.toLowerCase(type) == 'a')
				title = normalizeParagraphContent(content)
						.replaceAll("\\n", "");
			switch (type) {
			case 'A':
			case 'B':
				list.add(normalizeParagraphContent(content));
				id++;
				break;
			}
		}

		private void outputSection(char type, String content) {
			switch (Character.toLowerCase(type)) {
			case '#':
				out.print("<pre>\n");
				out.print(text2html(normalizePreformattedContent(content)));
				out.print("</pre>\n");
				break;
			case 'a':
			case 'b':
				outputHeader(type,
						text2html(normalizeParagraphContent(content)));
				break;
			case 'p':
				out.print("<p>\n");
				out.print(refillParagraph(
						text2html(normalizeParagraphContent(content)), PARLEN));
				out.print("</p>\n");
				break;
			case 'i':
				out.print("<p><em> ");
				out.print(text2html(normalizeParagraphContent(content)));
				out.print(" </em></p>\n");
				break;
			}
		}

		public void section(char type, String content) {
			if (pass == 0)
				analyzeSection(type, content);
			else
				outputSection(type, content);
		}

		public void end(int pass) throws Exception {
			if (pass == 0)
				return;
			out.print(epub ? epubEnd : htmlEnd);
			out.close();
		}
	}

	public static class TextProcessor implements Processor {

		private PrintStream out;

		public void setOutputStream(OutputStream out) {
			this.out = new PrintStream(out);
		}

		public int getNumberOfPasses() {
			return 1;
		}

		public void begin(int pass) throws Exception {
		}

		private void rep(String s) {
			for (int i = 0; i < s.length(); i++)
				out.print('=');
			out.println();
		}

		private void outputHeader(char type, String content) {
			if (Character.toLowerCase(type) == 'a')
				rep(content);
			out.println(content);
			rep(content);
		}

		public void section(char type, String content) {
			switch (Character.toLowerCase(type)) {
			case '#':
				out.print(normalizePreformattedContent(content));
				break;
			case 'a':
			case 'b':
				outputHeader(type, normalizeParagraphContent(content));
				break;
			case 'p':
				out.print(refillParagraph(normalizeParagraphContent(content),
						PARLEN));
				break;
			case 'i':
				out.print("* ");
				out.print(normalizeParagraphContent(content));
				out.println();
				break;
			}
			out.println();
		}

		public void end(int pass) throws Exception {
			out.close();
		}
	}

	public static class DATProcessor implements Processor {

		private OutputStream out;

		public void setOutputStream(OutputStream out) {
			this.out = out;
		}

		public int getNumberOfPasses() {
			return 1;
		}

		public void begin(int pass) throws Exception {
		}

		public void section(char type, String content) {
			try {
				out.write((byte) type);
				switch (Character.toLowerCase(type)) {
				case '#':
					out.write(normalizePreformattedContent(content).getBytes(
							"US-ASCII"));
					break;
				case 'a':
				case 'b':
				case 'p':
				case 'i':
					out.write(normalizeParagraphContent(content).getBytes(
							"US-ASCII"));
					break;
				default:
					throw new Exception("Invalid section type " + type);
				}
				out.write((byte) 0);
			} catch (Exception e) {
				throw new RuntimeException(e);
			}
		}

		public void end(int pass) throws Exception {
			out.close();
		}
	}

	private static void parseFile(BufferedReader in, Processor proc)
			throws Exception {
		StringBuilder builder = new StringBuilder();
		String line;
		char type = 0;
		boolean inParagraph = false;
		while ((line = in.readLine()) != null) {
			if (line.length() == 0) {
				if (!inParagraph)
					continue;
				proc.section(type, builder.toString());
				type = 0;
				inParagraph = false;
				builder.setLength(0);
			} else {
				if (!inParagraph) {
					inParagraph = true;
					char t = line.charAt(0);
					if (t != ' ' && (t < '0' || t > '9')
							&& line.charAt(1) == ' ') {
						type = t;
						line = line.substring(2);
					} else {
						type = '#';
					}
				}
				builder.append(line);
				builder.append('\n');
			}
		}
		if (builder.length() > 0) {
			proc.section(type, builder.toString());
		}
	}

	public static void process(File inFile, File outFile, Processor proc)
			throws Exception {
		for (int pass = 0; pass < proc.getNumberOfPasses(); pass++) {
			BufferedReader in = new BufferedReader(new FileReader(inFile));
			BufferedOutputStream out = new BufferedOutputStream(
					new FileOutputStream(outFile));
			try {
				proc.setOutputStream(out);
				proc.begin(pass);
				parseFile(in, proc);
				proc.end(pass);
			} finally {
				in.close();
			}
		}
	}

	public static void main(String[] args) throws Exception {
		File inFile = null;
		int i;
		for (i = 0; i < args.length; i++) {
			if (args[i].charAt(0) == '-') {
				String a = args[i];
				File outFile = new File(args[++i]);
				if (a.equals("-txt"))
					process(inFile, outFile, new TextProcessor());
				else if (a.equals("-html"))
					process(inFile, outFile, new HTMLProcessor(false));
				else if (a.equals("-epub"))
					process(inFile, outFile, new HTMLProcessor(true));
				else if (a.equals("-dat"))
					process(inFile, outFile, new DATProcessor());
				else
					throw new Exception("Unknown format option " + a);
			} else {
				inFile = new File(args[i]);
			}
		}
	}
}
