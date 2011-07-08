/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "AboutData.h"

#include "SurfacePool.h"
#include "WebKitVersion.h"
#include "wtf/Platform.h"

#include <osndk_version.h>
#include <sys/utsname.h>

namespace WebCore {

const char WEBKITCREDITS[]=
"<div class=\"about\"> "
"<h3>Acknowledgments</h3> "
"<p><b>The BlackBerry Browser contains portions of the WebKit Open Source "
"Project (including portions from the KDE project) and JavaScriptCore Project "
"(including portions from the KJS project). WebKit, WebCore and JavaScriptCore "
"contain software components licensed under the BSD license and software "
"components licensed under the GNU Library General Public License (LGPL) "
"versions 2.0 and 2.1.  Please refer to the individual software component "
"files to determine the license that applies to that particular component. "
"You can obtain a copy of the complete machine-readable source code of the LGPL "
"components (for no charge except for the costs associated with the media, "
"shipping and handling) upon written request to RIM.</b></p> "
"<p>Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, "
"2006, 2007, 2008, 2009, 2010 various contributors as noted below.</p> "
"<p>Adam Barth, Adam Dingle, Alex Mathews, Alex Milowski, Alexander Kellett, "
"Alexey Proskuryakov, Allan Sandfeld Jensen, Alp Toker, Anders Carlsson, "
"Andrea Anzani, Andrew Wellington, Anthony Ricaud, Antonio Gomes, "
"Antti Koivisto, Appcelerator Inc., Apple, Inc., Arno Renevier, Bjoern Graf, "
"Bobby Powers, Brent Fulgham, Cameron McCormack, Cameron Zwarich, "
"Charles Samuels, Charlie Bozeman, Christian Dywan, Christopher J. Madsen, "
"Collabora Ltd., Collin Jackson, Company 100 Inc., Cyrus Patel, Daniel Bates, "
"Daniel Molkentin, Daniel Veillard, David Levin, David Smith, Dawit Alemayehu, "
"Diego Escalante Urrelo, Diego Hidalgo C. Gonzalez, "
"Digital Equipment Corporation, Dirk Mueller, Dirk Schulze, Dominik R&ouml;ttsches, "
"Don Gibson, Emmanuel Pacaud, Eric Seidel, Ericsson AB, Feng Yuan, "
"Franois Sausset, Frederik Holljen, Free Software Foundation, Inc., "
"Frerich Raabe, Friedemann Kleint, George Staikos, Girish Ramakrishnan, "
"Google Inc., Gordon Freeman, Graham Dennis, Gustavo Noronha Silva, "
"Harri Porten, Henry Mason, Hiroyuki Ikezoe, Holger Hans Peter Freyther, "
"INdT Instituto Nokia de Tecnologia, Ian C. Bullard, Igalia S.L., "
"International Business Machines Corporation, Jakub Wieczorek, "
"James G. Speth, Jan Michael Alonzo, Jeff Schiller, Jian Li, Jim Nelson, "
"Joanmarie Diggs, Joel Stanley, John E. Bossom, John Kjellberg, Jon Shier, "
"Jonas Witt, Joseph Pecoraro, Julien Chaffraix, J&uuml;rg Billeter, Justin Haygood, "
"Kakai, Inc., Kelvin W Sherlock, Kenneth Rohde Christiansen, Kevin Ollivier, "
"Kevin Watters, Kimmo Kinnunen, Kouhei Sutou, Krzysztof Kowalczyk, Lars Knoll, "
"Luca Bruno, Lucent Technologies, Luke Kenneth Casson Leighton, "
"MIPS Technologies, Inc., Makoto Matsumoto and Takuji Nishimura, "
"Maks Orlovich, Maksim Orlovich, Martin Jones, Martin Robinson, Martin Soto, "
"Matt Lilek, Maxime Simon, Michael Emmel, Michelangelo De Simone, "
"Movial Creative Technologies Inc., Netscape Communications Corporation, "
"Nicholas Shanks, Nikolas Zimmermann, "
"Nokia Corporation and/or its subsidiary(-ies), Novell Inc., Nuanti Ltd., "
"Oliver Hunt, OpenedHand, Patrick Gansterer, Paul Pedriana, Peter Kelly, "
"Peter Laufenberg, Pioneer Research Center USA, Inc., "
"ProFUSION embedded systems, Red Hat, Inc, Research In Motion Limited, "
"Rob Buis, Robert Hogan, Ryan Leavengood, Samsung Electronics, Samuel Weinig, "
"Sebastian Drge, Simon Hausmann, Staikos Computing Services Inc., "
"Stefan Schimanski, Stephan Amus, Symantec Corporation, The Open Group, "
"Thomas Broyer, Tieto Corporation, Tobias Anton, Tony Chang, Torben Weis, "
"Torch Mobile (Beijing) Co. Ltd., Torch Mobile Inc., Trolltech ASA, "
"Ulrich Drepper, University of Cambridge, University of Szeged, "
"Vaclav Slavik, Waldo Bastian, Xan Lopez, Zack Rusin, Zan Dobersek, "
"Zoltan Herczeg, mozilla.org, 280 North Inc. </p> "
"<ol><li> The KHTML software is released under the GNU Library General Public License Version 2.</li></br> "
"<li>The KJS software is released under the GNU Library General Public License Version 2. "
"The authors also thank the following people for their help: Richard "
"Moore, Daegeun Lee, Marco Pinelli, and Christian Kirsh.</li></br> "
"<li> The KCanvas, KDOM, and KSVG2 software is released under the GNU Library General "
"Public License Version 2. </li></br> "
"</ol> "
"<p>For portions licensed under the BSD license:</br>"
"<p>Redistribution and use in source and binary forms, with or without "
"modification, are permitted provided that the following conditions "
"are met:</p>"
"<ul style=\"list-style:decimal\"> "
"<li>Redistributions of source code must retain the above copyright "
"notice, this list of conditions and the following disclaimer.</li></br> "
"<li>Redistributions in binary form must reproduce the above copyright "
"notice, this list of conditions and the following disclaimer in the "
"documentation and/or other materials provided with the distribution.</li></br> "
"<li>Neither the name of Apple Computer, Inc. (\"Apple\") nor the names of "
"its contributors may be used to endorse or promote products derived "
"from this software without specific prior written permission.</li> "
"</ul>"
"THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS \"AS IS\" AND ANY "
"EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED "
"WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE "
"DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY "
"DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES "
"(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; "
"LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND "
"ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT "
"(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF "
"THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</p>"
"<p> GNU LESSER GENERAL PUBLIC LICENSE<br> "
"Version 2.1, February 1999</p> "
"<p> Copyright (C) 1991, 1999 Free Software Foundation, Inc.</p> "
"<p> 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA</p> "
"<p> Everyone is permitted to copy and distribute verbatim copies "
"of this license document, but changing it is not allowed. </p> "
"<p>[This is the first released version of the Lesser GPL. It also counts "
"as the successor of the GNU Library Public License, version 2, hence "
"the version number 2.1.] </p> "
"<p> Preamble</p> "
"<p> The licenses for most software are designed to take away your "
"freedom to share and change it. By contrast, the GNU General Public Licenses are intended to guarantee your freedom to share and change "
"free software--to make sure the software is free for all its users.</p> "
"<p> This license, the Lesser General Public License, applies to some specially designated software packages--typically libraries--of the "
"Free Software Foundation and other authors who decide to use it. You "
"can use it too, but we suggest you first think carefully about whether this license or the ordinary General Public License is the better "
"strategy to use in any particular case, based on the explanations below.</p> "
"<p> When we speak of free software, we are referring to freedom of use, not price. Our General Public Licenses are designed to make sure that "
"you have the freedom to distribute copies of free software (and charge "
"for this service if you wish); that you receive source code or can get it if you want it; that you can change the software and use pieces of "
"it in new free programs; and that you are informed that you can do "
"these things.</p> "
"<p> To protect your rights, we need to make restrictions that forbid distributors to deny you these rights or to ask you to surrender these "
"rights. These restrictions translate to certain responsibilities for "
"you if you distribute copies of the library or if you modify it. </p> "
"<p> For example, if you distribute copies of the library, whether gratis "
"or for a fee, you must give the recipients all the rights that we gave "
"you. You must make sure that they, too, receive or can get the source code. If you link other code with the library, you must provide "
"complete object files to the recipients, so that they can relink them "
"with the library after making changes to the library and recompiling it. And you must show them these terms so they know their rights.</p> "
"<p> We protect your rights with a two-step method: (1) we copyright the "
"library, and (2) we offer you this license, which gives you legal permission to copy, distribute and/or modify the library.</p> "
"<p> To protect each distributor, we want to make it very clear that "
"there is no warranty for the free library. Also, if the library is "
"modified by someone else and passed on, the recipients should know "
"that what they have is not the original version, so that the original "
"author's reputation will not be affected by problems that might be introduced by others.</p> "
"<p> Finally, software patents pose a constant threat to the existence of "
"any free program. We wish to make sure that a company cannot "
"effectively restrict the users of a free program by obtaining a restrictive license from a patent holder. Therefore, we insist that "
"any patent license obtained for a version of the library must be "
"consistent with the full freedom of use specified in this license. </p> "
"<p> Most GNU software, including some libraries, is covered by the "
"ordinary GNU General Public License. This license, the GNU Lesser "
"General Public License, applies to certain designated libraries, and is quite different from the ordinary General Public License. We use "
"this license for certain libraries in order to permit linking those "
"libraries into non-free programs. </p> "
"<p> When a program is linked with a library, whether statically or using "
"a shared library, the combination of the two is legally speaking a "
"combined work, a derivative of the original library. The ordinary General Public License therefore permits such linking only if the "
"entire combination fits its criteria of freedom. The Lesser General "
"Public License permits more lax criteria for linking other code with the library.</p> "
"<p> We call this license the \"Lesser\" General Public License because it "
"does Less to protect the user's freedom than the ordinary General Public License. It also provides other free software developers Less "
"of an advantage over competing non-free programs. These disadvantages "
"are the reason we use the ordinary General Public License for many libraries. However, the Lesser license provides advantages in certain "
"special circumstances.</p> "
"<p> For example, on rare occasions, there may be a special need to "
"encourage the widest possible use of a certain library, so that it becomes a de-facto standard. To achieve this, non-free programs must be "
"allowed to use the library. A more frequent case is that a free "
"library does the same job as widely used non-free libraries. In this case, there is little to gain by limiting the free library to free "
"software only, so we use the Lesser General Public License.</p> "
"<p> In other cases, permission to use a particular library in non-free programs enables a greater number of people to use a large body of "
"free software. For example, permission to use the GNU C Library in "
"non-free programs enables many more people to use the whole GNU operating system, as well as its variant, the GNU/Linux operating "
"system.</p> "
"<p> Although the Lesser General Public License is Less protective of the "
"users' freedom, it does ensure that the user of a program that is linked with the Library has the freedom and the wherewithal to run "
"that program using a modified version of the Library.</p> "
"<p> The precise terms and conditions for copying, distribution and modification follow. Pay close attention to the difference between a "
"\"work based on the library\" and a \"work that uses the library\". The "
"former contains code derived from the library, whereas the latter must be combined with the library in order to run.</p> "
"<p> GNU LESSER GENERAL PUBLIC LICENSE<br> "
"TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION </p> "
"<ol start=\"0\"> "
"<li>This License Agreement applies to any software library or other "
"program which contains a notice placed by the copyright holder or "
"other authorized party saying it may be distributed under the terms of this Lesser General Public License (also called \"this License\"). "
"Each licensee is addressed as \"you\". "
"<p> A \"library\" means a collection of software functions and/or data prepared so as to be conveniently linked with application programs "
"(which use some of those functions and data) to form executables.</p> "
"<p> The \"Library\", below, refers to any such software library or work which has been distributed under these terms. A \"work based on the "
"Library\" means either the Library or any derivative work under "
"copyright law: that is to say, a work containing the Library or a portion of it, either verbatim or with modifications and/or translated "
"straightforwardly into another language. (Hereinafter, translation is "
"included without limitation in the term \"modification\".) </p> "
"<p> \"Source code\" for a work means the preferred form of the work for "
"making modifications to it. For a library, complete source code means "
"all the source code for all modules it contains, plus any associated</p> "
"<p>interface definition files, plus the scripts used to control compilation "
"and installation of the library.</p> "
"<p> Activities other than copying, distribution and modification are not covered by this License; they are outside its scope. The act of "
"running a program using the Library is not restricted, and output from "
"such a program is covered only if its contents constitute a work based on the Library (independent of the use of the Library in a tool for "
"writing it). Whether that is true depends on what the Library does "
"and what the program that uses the Library does.</p> "
"</li> "
"<li>You may copy and distribute verbatim copies of the Library's "
"complete source code as you receive it, in any medium, provided that "
"you conspicuously and appropriately publish on each copy an "
"appropriate copyright notice and disclaimer of warranty; keep intact "
"all the notices that refer to this License and to the absence of any "
"warranty; and distribute a copy of this License along with the Library. "
"<p> You may charge a fee for the physical act of transferring a copy, "
"and you may at your option offer warranty protection in exchange for a "
"fee.</p> "
"</li> "
"<li>You may modify your copy or copies of the Library or any portion "
"of it, thus forming a work based on the Library, and copy and "
"distribute such modifications or work under the terms of Section 1 above, provided that you also meet all of these conditions: "
"<ul style=\"list-style:lower-alpha\"> "
"<li>The modified work must itself be a software library.</li> "
"<li>You must cause the files modified to carry prominent notices stating that you changed the files and the date of any change.</li> "
"<li>You must cause the whole of the work to be licensed at no "
"charge to all third parties under the terms of this License.</li> "
"<li> If a facility in the modified Library refers to a function or a "
"table of data to be supplied by an application program that uses "
"the facility, other than as an argument passed when the facility is invoked, then you must make a good faith effort to ensure that, "
"in the event an application does not supply such function or "
"table, the facility still operates, and performs whatever part of its purpose remains meaningful. "
"<p> (For example, a function in a library to compute square roots has "
"a purpose that is entirely well-defined independent of the application. Therefore, Subsection 2d requires that any "
"application-supplied function or table used by this function must "
"be optional: if the application does not supply it, the square root function must still compute square roots.)</p> "
"</li> "
"</ul> "
"<p>These requirements apply to the modified work as a whole. If "
"identifiable sections of that work are not derived from the Library, and can be reasonably considered independent and separate works in "
"themselves, then this License, and its terms, do not apply to those "
"sections when you distribute them as separate works. But when you distribute the same sections as part of a whole which is a work based "
"on the Library, the distribution of the whole must be on the terms of "
"this License, whose permissions for other licensees extend to the entire whole, and thus to each and every part regardless of who wrote "
"it.</p> "
"<p>Thus, it is not the intent of this section to claim rights or contest "
"your rights to work written entirely by you; rather, the intent is to exercise the right to control the distribution of derivative or "
"collective works based on the Library.</p> "
"<p>In addition, mere aggregation of another work not based on the Library "
"with the Library (or with a work based on the Library) on a volume of "
"a storage or distribution medium does not bring the other work under "
"the scope of this License. </p> "
"</li> "
"<li> You may opt to apply the terms of the ordinary GNU General Public "
"License instead of this License to a given copy of the Library. To do "
"this, you must alter all the notices that refer to this License, so that they refer to the ordinary GNU General Public License, version 2, "
"instead of to this License. (If a newer version than version 2 of the "
"ordinary GNU General Public License has appeared, then you can specify that version instead if you wish.) Do not make any other change in "
"these notices. "
"<p> Once this change is made in a given copy, it is irreversible for "
"that copy, so the ordinary GNU General Public License applies to all subsequent copies and derivative works made from that copy.</p> "
"<p> This option is useful when you wish to copy part of the code of "
"the Library into a program that is not a library. </p> "
"</li> "
"<li>You may copy and distribute the Library (or a portion or "
"derivative of it, under Section 2) in object code or executable form "
"under the terms of Sections 1 and 2 above provided that you accompany it with the complete corresponding machine-readable source code, which "
"must be distributed under the terms of Sections 1 and 2 above on a "
"medium customarily used for software interchange. "
"<p> If distribution of object code is made by offering access to copy "
"from a designated place, then offering equivalent access to copy the "
"source code from the same place satisfies the requirement to distribute the source code, even though third parties are not "
"compelled to copy the source along with the object code.</p> "
"</li> "
"<li>A program that contains no derivative of any portion of the Library, but is designed to work with the Library by being compiled or "
"linked with it, is called a \"work that uses the Library\". Such a "
"work, in isolation, is not a derivative work of the Library, and "
"therefore falls outside the scope of this License. "
"<p> However, linking a \"work that uses the Library\" with the Library "
"creates an executable that is a derivative of the Library (because it "
"contains portions of the Library), rather than a \"work that uses the "
"library\". The executable is therefore covered by this License. "
"Section 6 states terms for distribution of such executables. </p> "
"<p> When a \"work that uses the Library\" uses material from a header file "
"that is part of the Library, the object code for the work may be a derivative work of the Library even though the source code is not.</p> "
"<p>Whether this is true is especially significant if the work can be "
"linked without the Library, or if the work is itself a library. The "
"threshold for this to be true is not precisely defined by law. </p> "
"<p> If such an object file uses only numerical parameters, data "
"structure layouts and accessors, and small macros and small inline "
"functions (ten lines or less in length), then the use of the object file is unrestricted, regardless of whether it is legally a derivative "
"work. (Executables containing this object code plus portions of the "
"Library will still fall under Section 6.) </p> "
"<p> Otherwise, if the work is a derivative of the Library, you may "
"distribute the object code for the work under the terms of Section 6. "
"Any executables containing that work also fall under Section 6, whether or not they are linked directly with the Library itself. </p> "
"</li> "
"<li>As an exception to the Sections above, you may also combine or "
"link a \"work that uses the Library\" with the Library to produce a work containing portions of the Library, and distribute that work "
"under terms of your choice, provided that the terms permit "
"modification of the work for the customer's own use and reverse engineering for debugging such modifications. "
"<p> You must give prominent notice with each copy of the work that the "
"Library is used in it and that the Library and its use are covered by this License. You must supply a copy of this License. If the work "
"during execution displays copyright notices, you must include the "
"copyright notice for the Library among them, as well as a reference directing the user to the copy of this License. Also, you must do one "
"of these things:</p> "
"<ul style=\"list-style:lower-alpha\"> "
"<li>Accompany the work with the complete corresponding "
"machine-readable source code for the Library including whatever changes were used in the work (which must be distributed under "
"Sections 1 and 2 above); and, if the work is an executable linked "
"with the Library, with the complete machine-readable \"work that uses the Library\", as object code and/or source code, so that the "
"user can modify the Library and then relink to produce a modified "
"executable containing the modified Library. (It is understood "
"that the user who changes the contents of definitions files in the "
"Library will not necessarily be able to recompile the application "
"to use the modified definitions.)</li> "
"<li>Use a suitable shared library mechanism for linking with the "
"Library. A suitable mechanism is one that (1) uses at run time a "
"copy of the library already present on the user's computer system, rather than copying library functions into the executable, and (2) "
"will operate properly with a modified version of the library, if "
"the user installs one, as long as the modified version is interface-compatible with the version that the work was made with.</li> "
"<li>Accompany the work with a written offer, valid for at "
"least three years, to give the same user the materials specified in Subsection 6a, above, for a charge no more "
"than the cost of performing this distribution.</li> "
"<li> If distribution of the work is made by offering access to copy from a designated place, offer equivalent access to copy the above "
"specified materials from the same place.</li> "
"<li> Verify that the user has already received a copy of these materials or that you have already sent this user a copy. </li> "
"</ul> "
"<p> For an executable, the required form of the \"work that uses the "
"Library\" must include any data and utility programs needed for "
"reproducing the executable from it. However, as a special exception, "
"the materials to be distributed need not include anything that is "
"normally distributed (in either source or binary form) with the major components (compiler, kernel, and so on) of the operating system on "
"which the executable runs, unless that component itself accompanies "
"the executable.</p> "
"<p> It may happen that this requirement contradicts the license "
"restrictions of other proprietary libraries that do not normally "
"accompany the operating system. Such a contradiction means you cannot "
"use both them and the Library together in an executable that you distribute.</p> "
"</li> "
"<li>You may place library facilities that are a work based on the "
"Library side-by-side in a single library together with other library "
"facilities not covered by this License, and distribute such a combined library, provided that the separate distribution of the work based on "
"the Library and of the other library facilities is otherwise "
"permitted, and provided that you do these two things: "
"<ul style=\"list-style:lower-alpha\"> "
"<li>Accompany the combined library with a copy of the same work "
"based on the Library, uncombined with any other library "
"facilities. This must be distributed under the terms of the Sections above. </li> "
"<li>Give prominent notice with the combined library of the fact "
"that part of it is a work based on the Library, and explaining "
"where to find the accompanying uncombined form of the same work.</li> "
"</ul> "
"</li> "
"<li>You may not copy, modify, sublicense, link with, or distribute "
"the Library except as expressly provided under this License. Any "
"attempt otherwise to copy, modify, sublicense, link with, or distribute the Library is void, and will automatically terminate your "
"rights under this License. However, parties who have received copies, "
"or rights, from you under this License will not have their licenses terminated so long as such parties remain in full compliance.</li> "
"<li>You are not required to accept this License, since you have not "
"signed it. However, nothing else grants you permission to modify or distribute the Library or its derivative works. These actions are "
"prohibited by law if you do not accept this License. Therefore, by "
"modifying or distributing the Library (or any work based on the Library), you indicate your acceptance of this License to do so, and "
"all its terms and conditions for copying, distributing or modifying "
"the Library or works based on it.</li> "
"<li>Each time you redistribute the Library (or any work based on the "
"Library), the recipient automatically receives a license from the "
"original licensor to copy, distribute, link with or modify the Library subject to these terms and conditions. You may not impose any further "
"restrictions on the recipients' exercise of the rights granted herein. "
"You are not responsible for enforcing compliance by third parties with this License. </li> "
"<li>If, as a consequence of a court judgment or allegation of patent "
"infringement or for any other reason (not limited to patent issues), "
"conditions are imposed on you (whether by court order, agreement or otherwise) that contradict the conditions of this License, they do not "
"excuse you from the conditions of this License. If you cannot "
"distribute so as to satisfy simultaneously your obligations under this License and any other pertinent obligations, then as a consequence you "
"may not distribute the Library at all. For example, if a patent "
"license would not permit royalty-free redistribution of the Library by all those who receive copies directly or indirectly through you, then "
"the only way you could satisfy both it and this License would be to "
"refrain entirely from distribution of the Library. "
"<p>If any portion of this section is held invalid or unenforceable under any "
"particular circumstance, the balance of the section is intended to apply, "
"and the section as a whole is intended to apply in other circumstances. </p> "
"<p>It is not the purpose of this section to induce you to infringe any "
"patents or other property right claims or to contest validity of any "
"such claims; this section has the sole purpose of protecting the integrity of the free software distribution system which is "
"implemented by public license practices. Many people have made "
"generous contributions to the wide range of software distributed through that system in reliance on consistent application of that "
"system; it is up to the author/donor to decide if he or she is willing "
"to distribute software through any other system and a licensee cannot impose that choice.</p> "
"<p>This section is intended to make thoroughly clear what is believed to "
"be a consequence of the rest of this License.</p> "
"</li> "
"<li>If the distribution and/or use of the Library is restricted in "
"certain countries either by patents or by copyrighted interfaces, the "
"original copyright holder who places the Library under this License may add "
"an explicit geographical distribution limitation excluding those countries, "
"so that distribution is permitted only in or among countries not thus "
"excluded. In such case, this License incorporates the limitation as if "
"written in the body of this License.</li> "
"<li>The Free Software Foundation may publish revised and/or new "
"versions of the Lesser General Public License from time to time. "
"Such new versions will be similar in spirit to the present version, but may differ in detail to address new problems or concerns. "
"Each version is given a distinguishing version number. If the Library "
"specifies a version number of this License which applies to it and "
"\"any later version\", you have the option of following the terms and "
"conditions either of that version or of any later version published by "
"the Free Software Foundation. If the Library does not specify a license version number, you may choose any version ever published by "
"the Free Software Foundation.</li> "
"<li> If you wish to incorporate parts of the Library into other free programs whose distribution conditions are incompatible with these, "
"write to the author to ask for permission. For software which is "
"copyrighted by the Free Software Foundation, write to the Free "
"Software Foundation; we sometimes make exceptions for this. Our "
"decision will be guided by the two goals of preserving the free status "
"of all derivatives of our free software and of promoting the sharing and reuse of software generally. "
"<p> NO WARRANTY</p> "
"</li> "
"<li>BECAUSE THE LIBRARY IS LICENSED FREE OF CHARGE, THERE IS NO "
"WARRANTY FOR THE LIBRARY, TO THE EXTENT PERMITTED BY APPLICABLE LAW. "
"EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR "
"OTHER PARTIES PROVIDE THE LIBRARY \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE<br> "
"IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR "
"PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE LIBRARY IS WITH YOU. SHOULD THE LIBRARY PROVE DEFECTIVE, YOU ASSUME "
"THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION. </li> "
"<li>IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN "
"WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY "
"AND/OR REDISTRIBUTE THE LIBRARY AS PERMITTED ABOVE, BE LIABLE TO YOU "
"FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE "
"LIBRARY (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING "
"RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES OR A FAILURE OF THE LIBRARY TO OPERATE WITH ANY OTHER SOFTWARE), EVEN IF "
"SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH "
"DAMAGES.</li> "
"</ol> "
"<p> END OF TERMS AND CONDITIONS</p> "
"<p> How to Apply These Terms to Your New Libraries</p> "
"<p> If you develop a new library, and you want it to be of the greatest "
"possible use to the public, we recommend making it free software that "
"everyone can redistribute and change. You can do so by permitting "
"redistribution under these terms (or, alternatively, under the terms of the "
"ordinary General Public License). </p> "
"<p> To apply these terms, attach the following notices to the library. It is "
"safest to attach them to the start of each source file to most effectively "
"convey the exclusion of warranty; and each file should have at least the \"copyright\" line and a pointer to where the full notice is found.</p> "
"<p> &lt;one line to give the library's name and a brief idea of what it does.&gt; "
"Copyright (C) &lt;year&gt; &lt;name of author&gt; </p> "
"<p> This library is free software; you can redistribute it and/or "
"modify it under the terms of the GNU Lesser General Public "
"License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.</p> "
"<p> This library is distributed in the hope that it will be useful, "
"but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU "
"Lesser General Public License for more details.</p> "
"<p> You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software "
"Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA</p> "
"<p>Also add information on how to contact you by electronic and paper mail. </p> "
"<p>You should also get your employer (if you work as a programmer) or your "
"school, if any, to sign a \"copyright disclaimer\" for the library, if "
"necessary. Here is a sample; alter the names: </p> "
"<p> Yoyodyne, Inc., hereby disclaims all copyright interest in the "
"library `Frob' (a library for tweaking knobs) written by James Random Hacker.</p> "
"<p> &lt;signature of Ty Coon&gt;, 1 April 1990</p> "
"<p> Ty Coon, President of Vice</p> "
"<p>That's all there is to it!</p></br> "
"<p>Copyright &copy; 1998-2000 Netscape Communications Corporation.</p>"
"<br>"
"<p></p>"
"<p>For WebCore/platform/Arena.[h|cpp], other contributors are: Nick Blievers &lt;nickb@adacel.com.au&gt;; Jeff Hostetler &lt;jeff@nerdone.com&gt;; Tom Rini &lt;trini@kernel.crashing.org&gt;; Raffaele Sena &lt;raff@netwinder.org&gt;;.</p>"
"<p></p>"
"<gt;p>For WebCore/rendering/RenderLayer.[h|cpp], other contributors are: Robert O'Callahan &lt;roc+@cs.cmu.edu&gt;; David Baron &lt;dbaron@fas.harvard.edu&gt;; Christian Biesinger &lt;cbiesinger@web.de&gt;; Randall Jesup &lt;rjesup@wgate.com&gt;; Roland Mainz &lt;roland.mainz@informatik.med.uni-giessen.de&gt;; Josh Soref &lt;timeless@mac.com&gt;; Boris Zbarsky &lt;bzbarsky@mit.edu&gt;;.</p>"
"<p></p>"
"<p>For WebCore/html/HTMLDocument.cpp, a contributor is: David Baron &lt;dbaron@fas.harvard.edu&gt;;.</p>"
"<p></p>"
"<p>This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, http://www.fsf.org/copyleft/lesser.html, or (at your option) any later version.</p>"
"<p></p>"
"<p>This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.</p>"
"<p></p>"
"<p>You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.</p>"
"<p></p>"
"<p>Alternatively, the contents of this file may be used under the terms of either the Mozilla Public License Version 1.1, found at http://www.mozilla.org/MPL/ (the \"MPL\") or the GNU General Public License Version 2.0, found at http://www.fsf.org/copyleft/gpl.html (the \"GPL\"), in which case the provisions of the MPL or the GPL are applicable instead of those above. If you wish to allow use of your version of this file only under the terms of one of those two licenses (the MPL or the GPL) and not to allow others to use your version of this file under the LGPL, indicate your decision by deleting the provisions above and replace them with the notice and other provisions required by the MPL or the GPL, as the case may be. If you do not delete the provisions above, a recipient may use your version of this file under any of the LGPL, the MPL or the GPL.</p></br>"
"<p>SpiderMonkey: Copyright &copy; 1998 Netscape Communications Corporation</p>"
"<p></p>"
"<p>DateMath.cpp: Copyright &copy; 1999-2000 Harri Porten &lt;porten@kde.org&gt;; Copyright &copy; 2006, 2007 Apple Inc. All rights reserved.</p>"
"<p></p>"
"<p>Version: MPL 1.1/GPL 2.0/LGPL 2.1</p>"
"<p></p>"
"<p>The contents of this file are subject to the Mozilla Public License Version 1.1 (the \"License\"); you may not use this file except in compliance with the License. You may obtain a copy of the License a http://www.mozilla.org/MPL/</p>"
"<p></p>"
"<p>Software distributed under the License is distributed on an \"AS IS\" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.</p>"
"<p></p>"
"<p>The Original Code is Mozilla Communicator client code, released March 31, 1998.</p>"
"<p></p>"
"<p>The Initial Developer of the Original Code is Netscape Communications Corporation. Portions created by the Initial Developer are Copyright (C) 1998 the Initial Developer. All Rights Reserved.</p>"
"<p></p>"
"<p>Alternatively, the contents of this file may be used under the terms of either of the GNU General Public License Version 2 or later (the \"GPL\"), or the GNU Lesser General Public License Version 2.1 or later (the \"LGPL\"), in which case the provisions of the GPL or the LGPL are applicable instead of those above. If you wish to allow use of your version of this file only under the terms of either the GPL or the LGPL, and not to allow others to use your version of this file under the terms of the MPL, indicate your decision by deleting the provisions above and replace them with the notice and other provisions required by the GPL or the LGPL. If you do not delete the provisions above, a recipient may use your version of this file under the terms of any one of the MPL, the GPL or the LGPL.</p></br>"
"PCRE"
"<p>Copyright (c) 1997-2005 University of Cambridge. All rights reserved.</p>"
"<p></p>"
"<p>Redistribution and use in source and binary forms, with or without</p>"
"<p>modification, are permitted provided that the following conditions are met:</p>"
"<p></p>"
"<p>* Redistributions of source code must retain the above copyright notice,</p>"
"<p>this list of conditions and the following disclaimer.</p>"
"<p></p>"
"<p>* Redistributions in binary form must reproduce the above copyright</p>"
"<p>notice, this list of conditions and the following disclaimer in the</p>"
"<p>documentation and/or other materials provided with the distribution.</p>"
"<p></p>"
"<p>* Neither the name of the University of Cambridge nor the name of Apple</p>"
"<p>Inc. nor the names of their contributors may be used to endorse or</p>"
"<p>promote products derived from this software without specific prior</p>"
"<p>written permission.</p>"
"<p></p>"
"<p>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"</p>"
"<p>AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE</p>"
"<p>IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE</p>"
"<p>ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE</p>"
"<p>LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR</p>"
"<p>CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF</p>"
"<p>SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS</p>"
"<p>INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN</p>"
"<p>CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)</p>"
"<p>ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE</p>"
"<p>POSSIBILITY OF SUCH DAMAGE.</p>"
"<hr>"
"<p>HarfBuzz was previously licensed under different licenses.  This was changed in January 2008.  If you need to relicense your old copies, consult the announcement of the license change on the internet. Other than that, each copy of HarfBuzz is licensed under the COPYING file included with it.  The actual license follows:</p>"
"<p>Permission is hereby granted, without written agreement and without license or royalty fees, to use, copy, modify, and distribute this software and its documentation for any purpose, provided that the above copyright notice and the following two paragraphs appear in all copies of this software.</p>"
"<p>IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</p>"
"THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN \"AS IS\" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.</p>"
"<hr>"
"<p>The GIFLIB distribution is Copyright (c) 1997  Eric S. Raymond</p>"
"<p>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:</p>"
"<p>The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.</p>"
"<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p>"
"<hr>"
"<p>Copyright (c) 1988-1996 Sam Leffler</p>"
"<p>Copyright (c) 1991-1996 Silicon Graphics, Inc.</p>"
"<p>Permission to use, copy, modify, distribute, and sell this software and its documentation for any purpose is hereby granted without fee, provided that (i) the above copyright notices and this permission notice appear in all copies of the software and related documentation, and (ii) the names of Sam Leffler and Silicon Graphics may not be used in any advertising or publicity relating to the software without the specific, prior written permission of Sam Leffler and Silicon Graphics.</p>"
"<p>THE SOFTWARE IS PROVIDED \"AS-IS\" AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.</p>"
"<p>IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.</p>"
"<hr>"
"<p>Copyright (C) 2001-2002 Thomas Broyer, Charlie Bozeman and Daniel Veillard.</p>"
"<p>All Rights Reserved.</p>"
"<p>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:</p>"
"<p>The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.</p>"
"<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p>"
"<p>Except as contained in this notice, the name of the authors shall not be used in advertising or otherwise to promote the sale, use or other dealings in this Software without prior written authorization from him.</p>"
"<hr>"
"<p>Copyright (C) 1998-2003 Daniel Veillard.  All Rights Reserved.</p>"
"<p>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:</p>"
"<p>The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.</p>"
"<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE DANIEL VEILLARD BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p>"
"<p>Except as contained in this notice, the name of Daniel Veillard shall not be used in advertising or otherwise to promote the sale, use or other dealings in this Software without prior written authorization from him.</p>"
"<hr>"
"<p>Copyright (C) 2000 Bjorn Reese and Daniel Veillard.</p>"
"<p>Copyright (C) 2003 Daniel Veillard.</p>"
"<p>Copyright (C) 2000 Gary Pennington and Daniel Veillard.</p>"
"<p>Copyright (C) 1998 Bjorn Reese and Daniel Stenberg.</p>"
"<p>Copyright (C) 2001 Bjorn Reese breese@users.sourceforge.net</p>"
"<p>Copyright (C) 2001 Bjorn Reese and Daniel Stenberg.</p>"
"<p>Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.</p>"
"<p>THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS AND CONTRIBUTORS ACCEPT NO RESPONSIBILITY IN ANY CONCEIVABLE MANNER.</p>"
"</div> "
;

String configPage()
{
    String page;

#if !PUBLIC_BUILD
#if COMPILER(RVCT)
    struct utsname info;
    if (uname(&info) != 0)
        return String();
#endif

    page = String("<html><head><title>BlackBerry Browser Configuration Information</title></head><body><h1>BlackBerry Browser Configuration Information</h1>")
    + "<h2>Compiler Information</h2><table>"
#if COMPILER(MSVC)
    + "<tr><td>Microsoft Visual C++</td><td>MSVC</td></tr>"
    + "<tr><td>_MSC_VER</td><td>" + String::number(_MSC_VER) + "</td></tr>"
    + "<tr><td>_MSC_FULL_VER</td><td>" + String::number(_MSC_FULL_VER) + "</td></tr>"
    + "<tr><td>_MSC_BUILD</td><td>" + String::number(_MSC_BUILD) + "</td></tr>"
#elif COMPILER(RVCT)
    + "<tr><td>ARM RealView Compiler Toolkit</td><td>RVCT</td></tr>"
    + "<tr><td>__ARMCC_VERSION</td><td>" + String::number(__ARMCC_VERSION) + "</td></tr>"
#if COMPILER(RVCT4GNU)
    + "<tr><td>RVCT 4+ in --gnu mode</td><td>1</td></tr>"
#endif
#endif
    ;

    page += String("</table><h2>OS Information</h2><table>")
    + "<tr><td>NDK_ABI_VERSION</td><td>" + String::number(NDK_ABI_VERSION) + "</td></tr>"
    + "<tr><td>_POSIX_</td><td>"
#if _POSIX_
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>USER_PROCESSES</td><td>"
#if USER_PROCESSES
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    ;

    page += String("</table><h2>CPU Information</h2><table>")
#if CPU(X86)
    + "<tr><td>X86</td><td></td></tr>"
#elif CPU(ARM)
    + "<tr><td>ARM</td><td></td></tr>"
    + "<tr><td>ARM_ARCH_VERSION</td><td>" + String::number(WTF_ARM_ARCH_VERSION) + "</td></tr>"
    + "<tr><td>THUMB_ARCH_VERSION</td><td>" + String::number(WTF_THUMB_ARCH_VERSION) + "</td></tr>"
    + "<tr><td>THUMB2</td><td>" + String::number(WTF_CPU_ARM_THUMB2) + "</td></tr>"
#endif
    + "<tr><td>Endianness</td><td>"
#if CPU(BIG_ENDIAN)
    + "big"
#elif CPU(MIDDLE_ENDIAN)
    + "middle"
#else
    + "little"
#endif
    + "</td></tr>"
    ;

    page += String("</table><h2>Platform Information</h2><table>")
    + "<tr><td>WebKit Version</td><td>" + String::number(WEBKIT_MAJOR_VERSION) + "." + String::number(WEBKIT_MINOR_VERSION) + "</td></tr>"
    + "<tr><td>Olympia</td><td>"
#if PLATFORM(OLYMPIA)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>EGL</td><td>"
#if PLATFORM(EGL)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>OpenVG</td><td>"
#if PLATFORM(OPENVG)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>JS_BLOCK_TOTAL_SIZE</td><td>" + String::number(JS_BLOCK_TOTAL_SIZE) + "</td></tr>"
    + "<tr><td>MOBILE</td><td>"
#if MOBILE
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>USE_SYSTEM_MALLOC</td><td>" + String::number(USE_SYSTEM_MALLOC) + "</td></tr>"
    + "<tr><td>Backingstore Tiles</td><td>" + String::number(Olympia::WebKit::SurfacePool::globalSurfacePool()->size()) + "</td></tr>"
    ;

    page += String("</table><h2>WebKit HAVE Information</h2><table>")
    + "<tr><td>ACCESSIBILITY</td><td>"
#if HAVE(ACCESSIBILITY)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>COMPUTED_GOTO</td><td>"
#if HAVE(COMPUTED_GOTO)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>ERRNO_H</td><td>"
#if HAVE(ERRNO_H)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>LANGINFO_H</td><td>"
#if HAVE(LANGINFO_H)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>MADV_FREE</td><td>"
#if HAVE(MADV_FREE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>MADV_FREE_REUSE</td><td>"
#if HAVE(MADV_FREE_REUSE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>MADV_DONTNEED</td><td>"
#if HAVE(MADV_DONTNEED)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>MERGESORT</td><td>"
#if HAVE(MERGESORT)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>MMAP</td><td>"
#if HAVE(MMAP)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>POSIX_MEMALIGN</td><td>"
#if HAVE(POSIX_MEMALIGN)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>PTHREAD_RWLOCK</td><td>"
#if HAVE(PTHREAD_RWLOCK)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>PTHREAD_SETNAME_NP</td><td>"
#if HAVE(PTHREAD_SETNAME_NP)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>READLINE</td><td>"
#if HAVE(READLINE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>RUNLOOP_TIMER</td><td>"
#if HAVE(RUNLOOP_TIMER)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SBRK</td><td>"
#if HAVE(SBRK)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SIGNAL_H</td><td>"
#if HAVE(SIGNAL_H)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>STRINGS_H</td><td>"
#if HAVE(STRINGS_H)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SYS_PARAM_H</td><td>"
#if HAVE(SYS_PARAM_H)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SYS_TIME_H</td><td>"
#if HAVE(SYS_TIME_H)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SYS_TIMEB_H</td><td>"
#if HAVE(SYS_TIMEB_H)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>VIRTUALALLOC</td><td>"
#if HAVE(VIRTUALALLOC)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>VIRTUAL_MEMORY</td><td>"
#if HAVE(VIRTUAL_MEMORY)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    ;

    page += String("</table><h2>WebKit USE Information</h2><table>")
    + "<tr><td>ACCELERATED_COMPOSITING</td><td>"
#if USE(ACCELERATED_COMPOSITING)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>EXTERNAL_CRASH</td><td>"
#if USE(EXTERNAL_CRASH)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>FONT_FAST_PATH</td><td>"
#if USE(FONT_FAST_PATH)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>GOOGLEURL</td><td>"
#if USE(GOOGLEURL)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>ICU_UNICODE</td><td>"
#if USE(ICU_UNICODE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>INTERPRETER</td><td>"
#if USE(INTERPRETER)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>JAVASCRIPTCORE_BINDINGS</td><td>"
#if USE(JAVASCRIPTCORE_BINDINGS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>JIT_STUB_ARGUMENT_VA_LIST</td><td>"
#if USE(JIT_STUB_ARGUMENT_VA_LIST)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>JSC</td><td>"
#if USE(JSC)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>JSVALUE32_64</td><td>"
#if USE(JSVALUE32_64)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>JSVALUE64</td><td>"
#if USE(JSVALUE64)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>LOCKFREE_THREADSAFESHARED</td><td>"
#if USE(LOCKFREE_THREADSAFESHARED)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>OLYMPIA_UNICODE</td><td>"
#if USE(OLYMPIA_UNICODE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>OLYMPIA_NATIVE_UNICODE</td><td>"
#if USE(OLYMPIA_NATIVE_UNICODE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>PLUGIN_HOST_PROCESS</td><td>"
#if USE(PLUGIN_HOST_PROCESS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>PTHREADS</td><td>"
#if USE(PTHREADS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>QUERY_PERFORMANCE_COUNTER</td><td>"
#if USE(QUERY_PERFORMANCE_COUNTER)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>V8</td><td>"
#if USE(V8)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    ;

    page += String("</table><h2>WebKit ENABLE Information</h2><table>")
    + "<tr><td>3D_CANVAS</td><td>"
#if ENABLE(3D_CANVAS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>3D_RENDERING</td><td>"
#if ENABLE(3D_RENDERING)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>APPLICATION_CACHE_DYNAMIC_ENTRIES</td><td>"
#if ENABLE(APPLICATION_CACHE_DYNAMIC_ENTRIES)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>ARCHIVE</td><td>"
#if ENABLE(ARCHIVE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>ASSEMBLER</td><td>"
#if ENABLE(ASSEMBLER)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>ASSEMBLER_WX_EXCLUSIVE</td><td>"
#if ENABLE(ASSEMBLER_WX_EXCLUSIVE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>BROKEN_DOM</td><td>"
#if ENABLE(BROKEN_DOM)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>CHANNEL_MESSAGING</td><td>"
#if ENABLE(CHANNEL_MESSAGING)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>CLIENT_BASED_GEOLOCATION</td><td>"
#if ENABLE(CLIENT_BASED_GEOLOCATION)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>CODEBLOCK_SAMPLING</td><td>"
#if ENABLE(CODEBLOCK_SAMPLING)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>CONTEXT_MENUS</td><td>"
#if ENABLE(CONTEXT_MENUS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>CSS_VARIABLES</td><td>"
#if ENABLE(CSS_VARIABLES)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>DASHBOARD_SUPPORT</td><td>"
#if ENABLE(DASHBOARD_SUPPORT)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>DATABASE</td><td>"
#if ENABLE(DATABASE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>DATAGRID</td><td>"
#if ENABLE(DATAGRID)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>DATALIST</td><td>"
#if ENABLE(DATALIST)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>DIRECTIONAL_PAD_NAVIGATION</td><td>"
#if ENABLE(DIRECTIONAL_PAD_NAVIGATION)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>DOM_STORAGE</td><td>"
#if ENABLE(DOM_STORAGE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>DRAG_SUPPORT</td><td>"
#if ENABLE(DRAG_SUPPORT)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>EVENTSOURCE</td><td>"
#if ENABLE(EVENTSOURCE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>EXPERIMENTAL_SINGLE_VIEW_MODE</td><td>"
#if ENABLE(EXPERIMENTAL_SINGLE_VIEW_MODE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>FAST_MALLOC_MATCH_VALIDATION</td><td>"
#if ENABLE(FAST_MALLOC_MATCH_VALIDATION)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>FAST_MOBILE_SCROLLING</td><td>"
#if ENABLE(FAST_MOBILE_SCROLLING)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>FILTERS</td><td>"
#if ENABLE(FILTERS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>FTPDIR</td><td>"
#if ENABLE(FTPDIR)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>GEOLOCATION</td><td>"
#if ENABLE(GEOLOCATION)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>ICONDATABASE</td><td>"
#if ENABLE(ICONDATABASE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>IGNORE_FIXED_STYLES</td><td>"
#if ENABLE(IGNORE_FIXED_STYLES)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>IMAGE_DECODER_DOWN_SAMPLING</td><td>"
#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>INSPECTOR</td><td>"
#if ENABLE(INSPECTOR)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>JAVASCRIPT_DEBUGGER</td><td>"
#if ENABLE(JAVASCRIPT_DEBUGGER)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>JIT</td><td>"
#if ENABLE(JIT)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>JIT_OPTIMIZE_CALL</td><td>"
#if ENABLE(JIT_OPTIMIZE_CALL)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>JIT_OPTIMIZE_METHOD_CALLS</td><td>"
#if ENABLE(JIT_OPTIMIZE_METHOD_CALLS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>JIT_OPTIMIZE_NATIVE_CALL</td><td>"
#if ENABLE(JIT_OPTIMIZE_NATIVE_CALL)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>JIT_OPTIMIZE_PROPERTY_ACCESS</td><td>"
#if ENABLE(JIT_OPTIMIZE_PROPERTY_ACCESS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>JSC_MULTIPLE_THREADS</td><td>"
#if ENABLE(JSC_MULTIPLE_THREADS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>JSC_ZOMBIES</td><td>"
#if ENABLE(JSC_ZOMBIES)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>MAC_JAVA_BRIDGE</td><td>"
#if ENABLE(MAC_JAVA_BRIDGE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>MATHML</td><td>"
#if ENABLE(MATHML)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>NETSCAPE_PLUGIN_API</td><td>"
#if ENABLE(NETSCAPE_PLUGIN_API)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>NOTIFICATIONS</td><td>"
#if ENABLE(NOTIFICATIONS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>OFFLINE_WEB_APPLICATIONS</td><td>"
#if ENABLE(OFFLINE_WEB_APPLICATIONS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>OLYMPIA_DEBUG_MEMORY</td><td>"
#if ENABLE(OLYMPIA_DEBUG_MEMORY)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>OLYMPIA_OBJECT_ALLOCATOR</td><td>"
#if ENABLE(OLYMPIA_OBJECT_ALLOCATOR)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>ON_FIRST_TEXTAREA_FOCUS_SELECT_ALL</td><td>"
#if ENABLE(ON_FIRST_TEXTAREA_FOCUS_SELECT_ALL)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>OPCODE_SAMPLING</td><td>"
#if ENABLE(OPCODE_SAMPLING)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>OPCODE_STATS</td><td>"
#if ENABLE(OPCODE_STATS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>OPENTYPE_SANITIZER</td><td>"
#if ENABLE(OPENTYPE_SANITIZER)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>ORIENTATION_EVENTS</td><td>"
#if ENABLE(ORIENTATION_EVENTS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>PAN_SCROLLING</td><td>"
#if ENABLE(PAN_SCROLLING)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>PERFECT_HASH_SIZE</td><td>"
#if ENABLE(PERFECT_HASH_SIZE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>PLUGIN_PACKAGE_SIMPLE_HASH</td><td>"
#if ENABLE(PLUGIN_PACKAGE_SIMPLE_HASH)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>PLUGIN_PROXY_FOR_VIDEO</td><td>"
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>QT_BEARER</td><td>"
#if ENABLE(QT_BEARER)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>REPAINT_THROTTLING</td><td>"
#if ENABLE(REPAINT_THROTTLING)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SAMPLING_COUNTERS</td><td>"
#if ENABLE(SAMPLING_COUNTERS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SAMPLING_FLAGS</td><td>"
#if ENABLE(SAMPLING_FLAGS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SAMPLING_THREADS</td><td>"
#if ENABLE(SAMPLING_THREADS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SCROLLBARS</td><td>"
#if ENABLE(SCROLLBARS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SHARED_SCRIPT</td><td>"
#if ENABLE(SHARED_SCRIPT)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SHARED_WORKERS</td><td>"
#if ENABLE(SHARED_WORKERS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SINGLE_THREADED</td><td>"
#if ENABLE(SINGLE_THREADED)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SQLITE</td><td>"
#if ENABLE(SQLITE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SVG</td><td>"
#if ENABLE(SVG)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SVG_ANIMATION</td><td>"
#if ENABLE(SVG_ANIMATION)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SVG_AS_IMAGE</td><td>"
#if ENABLE(SVG_AS_IMAGE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SVG_EXPERIMENTAL_FEATURES</td><td>"
#if ENABLE(SVG_EXPERIMENTAL_FEATURES)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SVG_FONTS</td><td>"
#if ENABLE(SVG_FONTS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SVG_FOREIGN_OBJECT</td><td>"
#if ENABLE(SVG_FOREIGN_OBJECT)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>SVG_USE</td><td>"
#if ENABLE(SVG_USE)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>TEXT_CARET</td><td>"
#if ENABLE(TEXT_CARET)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>TOUCH_EVENTS</td><td>"
#if ENABLE(TOUCH_EVENTS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>VIDEO</td><td>"
#if ENABLE(VIDEO)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>VIEWPORT_REFLOW</td><td>"
#if ENABLE(VIEWPORT_REFLOW)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>WBXML</td><td>"
#if ENABLE(WBXML)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>WCSS</td><td>"
#if ENABLE(WCSS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>WEB_SOCKETS</td><td>"
#if ENABLE(WEB_SOCKETS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>WML</td><td>"
#if ENABLE(WML)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>WORKERS</td><td>"
#if ENABLE(WORKERS)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>WREC</td><td>"
#if ENABLE(WREC)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>XBL</td><td>"
#if ENABLE(XBL)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>XHTMLMP</td><td>"
#if ENABLE(XHTMLMP)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>XPATH</td><td>"
#if ENABLE(XPATH)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>XSLT</td><td>"
#if ENABLE(XSLT)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>YARR</td><td>"
#if ENABLE(YARR)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "<tr><td>YARR_JIT</td><td>"
#if ENABLE(YARR_JIT)
    + "1"
#else
    + "0"
#endif
    + "</td></tr>"
    + "</table>"
    ;

#if COMPILER(RVCT)
    page += String("<h2>Process Information</h2><table>")
    + "<tr><td>sysname</td><td>" + info.sysname + "</td></tr>"
    + "<tr><td>nodename</td><td>" + info.nodename + "</td></tr>"
    + "<tr><td>release</td><td>" + info.release + "</td></tr>"
    + "<tr><td>version</td><td>" + info.version + "</td></tr>"
    + "<tr><td>machine</td><td>" + info.machine + "</td></tr>"
    + "<tr><td>Version</td><td>" + String::number(info.Version) + "</td></tr>"
    + "<tr><td>BuildUserName</td><td>" + info.BuildUserName + "</td></tr>"
    + "<tr><td>BuildDate</td><td>" + info.BuildDate + "</td></tr>"
    + "<tr><td>BuildTime</td><td>" + info.BuildTime + "</td></tr>"
    + "<tr><td>DeviceString</td><td>" + info.DeviceString + "</td></tr>"
    + "<tr><td>HardwareID</td><td>" + String::number(info.HardwareID) + "</td></tr>"
    + "<tr><td>OSVersion</td><td>" + String::number(info.OSVersion) + "</td></tr>"
    + "<tr><td>FatFSVersion</td><td>" + String::number(info.FatFSVersion) + "</td></tr>"
    + "<tr><td>RadioCodeLinked</td><td>" + String::number(info.RadioCodeLinked) + "</td></tr>"
    + "<tr><td>AppLinkVersion</td><td>" + String::number(info.AppLinkVersion) + "</td></tr>"
    + "<tr><td>PackageVersionString</td><td>" + info.PackageVersionString + "</td></tr>"
    + "<tr><td>DebugSetting</td><td>" + String::number(info.DebugSetting)+ "</td></tr>"
    + "</table>"
    ;
#endif

    page += String("</body></html>");
#endif

    return page;
}

}
