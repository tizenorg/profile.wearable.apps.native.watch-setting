# for i18n

SET(POFILES
			ar.po
			as.po
			bn.po
			fa.po
			tl.po
			gu.po
			he.po
			hi.po
			id.po
			ja_JP.po
			km.po
			kn.po
			lo.po
			ml.po
			mr.po
			ms.po
			my.po
			or.po
			pa.po
			si.po
			sq.po
			ta.po
			te.po
			th.po
			ur.po
			vi.po
			zh_HK.po
			zh_TW.po
			en_PH.po
			ga.po
			mk.po
			zh_CN.po
			az.po
			bg.po
			ca.po
			cs.po
			da.po
			de.po
			el_GR.po
			en.po
			en_US.po
			es_ES.po
			es_US.po
			et.po
			eu.po
			fi.po
			fr_CA.po
			fr.po
			gl.po
			hr.po
			hu.po
			hy.po
			is.po
			it_IT.po
			ka.po
			kk.po
			ko_KR.po
			lt.po
			lv.po
			nb.po
			ne.po
			nl.po
			pl.po
			pt_BR.po
			pt_PT.po
			ro.po
			ru_RU.po
			sk.po
			sl.po
			sr.po
			sv.po
			tr_TR.po
			uk.po
			uz.po
)

SET(MSGFMT "/usr/bin/msgfmt")

FOREACH(pofile ${POFILES})
	SET(pofile ${CMAKE_CURRENT_SOURCE_DIR}/${pofile})
	MESSAGE("PO: ${pofile}")
	GET_FILENAME_COMPONENT(absPofile ${pofile} ABSOLUTE)
	GET_FILENAME_COMPONENT(lang ${absPofile} NAME_WE)
	SET(moFile ${CMAKE_CURRENT_BINARY_DIR}/${lang}.mo)
	ADD_CUSTOM_COMMAND(
			OUTPUT ${moFile}
			COMMAND ${MSGFMT} -o ${moFile} ${absPofile}
			DEPENDS ${absPofile}
	)
	INSTALL(FILES ${moFile}
			DESTINATION ${LOCALEDIR}/${lang}/LC_MESSAGES RENAME ${PROJECT_NAME}.mo)
	SET(moFiles ${moFiles} ${moFile})
ENDFOREACH(pofile)

MESSAGE(".mo files: ${moFiles}")
ADD_CUSTOM_TARGET(po ALL DEPENDS ${moFiles})
