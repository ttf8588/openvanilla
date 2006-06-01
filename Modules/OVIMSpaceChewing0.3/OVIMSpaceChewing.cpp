// OVIMChewing03.cpp
// This package is released under the Artistic License,
// please refer to LICENSE.txt for the terms of use and distribution

// #define OV_DEBUG

#include <stdio.h> 
#include <stdlib.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <OpenVanilla/OpenVanilla.h>
#include <OpenVanilla/OVLibrary.h>
#include <OpenVanilla/OVUtility.h>
// #include <ChewingPP/Chewingpp.h>
#include "chewingio.h"
#include "mod_aux.h"

int ChewingFileExist(const char *path, const char *file) {
    char buf[PATH_MAX];
    sprintf(buf, "%s/%s", path, file);
    struct stat st;
    if (stat(buf, &st)) return 0;   // doesn't exist
    return 1;
}

int ChewingCheckData(const char *path) {
    char *files[5]={"ch_index.dat", "dict.dat", "fonetree.dat", "ph_index.dat",  "us_freq.dat"};
    for (int i=0; i<5; i++) if (!ChewingFileExist(path, files[i])) return 0;
    return 1;
}

class OVIMChewing03;

class OVIMChewing03Context : public OVInputMethodContext 
{
public:
    OVIMChewing03Context(OVIMChewing03 *p, ChewingContext *ctx) {
        p=parent;
        im=ctx;
    }
	
	virtual void start(OVBuffer *key, OVCandidate *textbar, OVService *srv) {	
	} 
    virtual void clear() { chewing_handle_Enter(im); }
    virtual void end() { chewing_handle_Enter(im); }
     
    virtual int keyEvent(OVKeyCode *key, OVBuffer *buf, OVCandidate *textbar, OVService *srv) {
        if(key->isCommand()) return 0;
        KeyPress(key,buf,textbar,srv);
        // if(im->KeystrokeIgnore()) return 0;
        CandidateWindow(textbar, srv);
        Redraw(buf, srv);
        return 1;
    }
  
protected:
    void KeyPress(OVKeyCode *key, OVBuffer *buf, OVCandidate *textbar, OVService *srv) {
        int k = key->code();
        Capslock(key,buf,textbar,srv);
        if(k == ovkSpace) {
            chewing_handle_Space(im);
        }
        else if (k == ovkLeft)   {
             chewing_handle_Left(im);
        }
        else if (k == ovkRight)  {
             chewing_handle_Right(im);
        }
        else if (k == ovkDown) { chewing_handle_Down(im); }
        else if (k == ovkEsc)  { chewing_handle_Esc(im);  }
        else if (k == ovkTab)  { chewing_handle_Tab(im);  }
        else if (k == ovkHome) { chewing_handle_Home(im); }
        else if (k == ovkEnd)  { chewing_handle_End(im); }
        else if (k == ovkDelete || k == ovkBackspace) { chewing_handle_Del(im) ; }
        else if (k == ovkReturn) { chewing_handle_Enter(im); }
        else { 
            DefaultKey(key,buf,textbar,srv);
        }
    }

    void DefaultKey(OVKeyCode *key, OVBuffer *buf, OVCandidate *textbar, OVService *srv) {
        if(key->isCtrl()) {
            if((key->code() >= '0') && (key->code() <= '9')) {
                chewing_handle_CtrlNum( im, (key->code()));
            }
            return;
        }
        chewing_handle_Default(im ,(key->isShift())?toupper(key->code()):tolower(key->code()));
    }

    void Capslock(OVKeyCode *key, OVBuffer *buf, OVCandidate *textbar, OVService *srv) {
        if(key->isCapslock()) {
            chewing_handle_Capslock( im );
        }
    }
    
    void Redraw(OVBuffer *buf, OVService *srv) {
        //const char *s1,*s2,*s3;
        
        /*
        if(im->CommitReady()) {
            const char *s = im->CommitStr();
            buf->clear()->append(s)->send();
        } */
        if ( chewing_commit_Check( im ) ) {
                const char *s = chewing_commit_String( im );
                buf->clear()->append(s)->send();
        }
        /*
        int ps=-1, pe=-1, ips=im->PointStart(), ipe=im->PointEnd();
        if (ips > -1 && ipe !=0) {
            if (ipe > 0) { ps=ips; pe=ps+ipe; }
            else { ps=ips+ipe; pe=ips; }
        } */        
       const char *s= chewing_buffer_String( im );
        buf->clear()->append(s)->update();
        // murmur("ips=%d, ipe=%d, ps=%d, pe=%d\n", ips, ipe, ps, pe);
        /*
        s1 = srv->toUTF8("big5", im->Buffer(0,im->CursorPos()-1));
        buf->clear()->append(s1);

        s2 = srv->toUTF8("big5", im->ZuinStr());
        buf->append(s2);
        
        s3 = srv->toUTF8("big5", im->Buffer(im->CursorPos()));
        buf->append(s3)->update(im->CursorPos(), ps, pe); */
        // murmur("==> %s%s%s",s1,s2,s3);
    }
    
    void CandidateWindow(OVCandidate *textbar, OVService *srv) {
        if(chewing_cand_TotalPage(im) > 0) {

           /* char s[64];
			char *ch, selkey; */
			int i=1;
			char str[ 20 ];
			char *cand_string;
            textbar->clear();
            chewing_cand_Enumerate( im );
	        while ( chewing_cand_hasNext( im ) ) {
	       	   if ( i == chewing_cand_ChoicePerPage( im ) )
	       	   	   break;
	       	   sprintf( str, "%d.", i );
	       	   textbar->append( str );
	       	   cand_string = chewing_cand_String( im );
	       	   sprintf( str, " %s ", cand_string );
	       	   textbar->append( str );
	       	   free( cand_string );
	       	   i++;
	       }
           /* sprintf(s," %d/%d",im->CurrentPage() + 1,im->TotalPage());
            textbar->append((char*)s); */
            textbar->update();
            textbar->show();
        }
        else {
            textbar->hide();
        }
    }

protected:
    OVIMChewing03 *parent;
    ChewingContext *im;
};


class OVIMChewing03 : public OVInputMethod {
public:
    OVIMChewing03() {
        ctx=NULL;
    }

    virtual ~OVIMChewing03() {
        delete ctx;
    }

    virtual int initialize(OVDictionary* l, OVService* s, const char* modulePath) {
        char chewingpath[PATH_MAX];
        char hashdir[PATH_MAX];
        ChewingConfigData config;
        
        // no more need to create hash dir ourselves
        sprintf(hashdir, "%s%s", s->userSpacePath(identifier()),
            s->pathSeparator());
        
        sprintf(chewingpath, "%sOVIMSpaceChewing03", modulePath);
        if (!ChewingCheckData(chewingpath)) {
            murmur("OVIMSpaceChewing: chewing data missing at %s", modulePath);
            return 0;
        }
        
        murmur ("OVIMSpaceChewing: initialize, chewing data=%s, userhash=%s", chewingpath, hashdir);
		
		// BECAUSE THE {SACRILEGIOUS WORDS HERE} libchewing HAS NO 
		// EXCEPTION HANDLING HERE (BLAME OLD C-style assert() !!)
		// WE HAVE TO DO ERROR CHECKING OURSELVES, OTHERWISE WE ARE
        // DOOMED IF CHEWING DATA DOESN'T EXIST. THIS MAKES OUR LIFE
        // HARD BUT WE SHOULD TRY NOT TO COMPLAIN
        // chew = new Chewing(chewingpath, hashdir);
        chewing_Init(chewingpath, hashdir);
        ctx = chewing_new();

        if(!l->keyExist("keyboardLayout")) l->setInteger("keyboardLayout", 0);
        chewing_set_KBType( ctx, l->getInteger("keyboardLayout"));
		//chew->SetKeyboardLayout(l->getInteger("keyboardLayout"));
		        
        config.selectAreaLen = 20;
        config.maxChiSymbolLen = 16;
        // int i;
        /* for ( i = 0; i < 10; i++ )
                config.selKey[ i ] = selKey_define[ i ]; */
        /* Enable configurations */
        chewing_Configure( ctx, &config );

        return 1;
    }

    virtual void update(OVDictionary* localconfig, OVService*) {
        chewing_set_KBType( ctx, localconfig->getInteger("keyboardLayout"));
//        chew->SetKeyboardLayout(localconfig->getInteger("keyboardLayout"));
    }

    virtual const char *identifier() {
        return "OVIMSpaceChewing03";
    }

    virtual const char *localizedName(const char *locale) {
        if (!strcasecmp(locale, "zh_TW")) return "酷音 0.3";
        if (!strcasecmp(locale, "zh_CN")) return "繁体酷音 0.3";
 	    return "Chewing (Smart Phonetics)";
    }

    virtual OVInputMethodContext* newContext() {
	    return new OVIMChewing03Context(this, ctx); 
    }
    
protected:
    ChewingContext *ctx;
};

// the module wrapper
OV_SINGLE_MODULE_WRAPPER(OVIMChewing03);