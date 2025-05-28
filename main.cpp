#include <windows.h>
#include <shlobj.h>
#include <iostream>
#include <string>
#include <vector>
#include <urlmon.h>
#include <filesystem>
#pragma comment(lib, "urlmon.lib")

std::wstring getUserPath() {
    PWSTR path = NULL;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &path);

    if (SUCCEEDED(hr)) {
        std::wcout << L"Home folder: " << path << std::endl;

        std::wstring userPath = path;
        
        CoTaskMemFree(path);
        return userPath;
    }
}

void downloadImages(const std::wstring path) {
    std::vector<std::wstring> imageUrls = {
        L"https://images.unsplash.com/photo-1506744038136-46273834b3fb",
        L"https://images.unsplash.com/photo-1523413651479-597eb2da0ad6",
        L"https://images.unsplash.com/photo-1470770841072-f978cf4d019e",
        L"https://images.unsplash.com/photo-1482192596544-9eb780fc7f66",
        L"https://images.unsplash.com/photo-1505761671935-60b3a7427bad",
        L"https://images.unsplash.com/photo-1508051123996-69f8caf4891e",
        L"https://images.unsplash.com/photo-1494526585095-c41746248156",
        L"https://images.unsplash.com/photo-1517423440428-a5a00ad493e8",
        L"https://images.unsplash.com/photo-1555685812-4b943f1cb0eb",
        L"https://images.unsplash.com/photo-1517841905240-472988babdf9",
        L"https://images.unsplash.com/photo-1446776811953-b23d57bd21aa",
        L"https://images.unsplash.com/photo-1500530855697-b586d89ba3ee",
        L"https://images.unsplash.com/photo-1462331940025-496dfbfc7564",
        L"https://images.unsplash.com/photo-1535909339361-9f8f8a67a3a1",
        L"https://images.unsplash.com/photo-1602524818701-93e9f116f9b2",
        L"https://images.unsplash.com/photo-1616401789115-541dcf49c1b1",
        L"https://images.unsplash.com/photo-1524995997946-a1c2e315a42f",
        L"https://images.unsplash.com/photo-1509021436665-8f07dbf5bf1d",
        L"https://images.unsplash.com/photo-1573497019119-44c7353e2105",
        L"https://images.unsplash.com/photo-1535713875002-d1d0cf377fde",
        L"https://images.unsplash.com/photo-1544005313-94ddf0286df2",
        L"https://images.unsplash.com/photo-1544006659-f0b21884ce1d",
        L"https://images.unsplash.com/photo-1518770660439-4636190af475",
        L"https://images.unsplash.com/photo-1517433456452-f9633a875f6f",
        L"https://images.unsplash.com/photo-1551033406-611cf9a28f73"
    };

    std::cout << "[+] Lade JPG Dateien herunter" << std::endl;
    for (size_t i = 0; i < imageUrls.size(); ++i) {
        std::wstring filename = path + L"\\Desktop\\" + std::to_wstring(i) + L".jpg";
        HRESULT hr = URLDownloadToFileW(NULL, imageUrls[i].c_str(), filename.c_str(), 0, NULL);
    }

    std::vector<std::wstring> excelFiles = {
    L"https://mmbb.law/wp-content/uploads/Digital-Assets-Worksheet-Password-List.xlsx",
    L"https://blog.testlodge.com/wp-content/uploads/2015/04/Importing-Test-Cases-Into-TestLodge_Assett.xlsx",
    L"https://openlmis.org/wp-content/uploads/2018/04/UAT_SampleScripts.xlsx",
    L"https://dms-media.ccplatform.net/content/download/158084/file/MFMP%20Release_1.3.0_Overview.xlsx",
    L"https://ccfcmaine.org/wp-content/uploads/2024/05/CCFC-WiFi-Points-V2.xlsx",
    L"https://www.fedramp.gov/assets/resources/documents/FedRAMP_Low_Security_Controls.xlsx",
    L"https://help.hostedftp.com/wp-content/uploads/2021/08/Sample-User-Import-Template.xlsx",
    L"https://services.umassglobal.edu/Banner/presentation/Strings_Properties_TEMPLATE.xlsx",
    L"https://forum-kobotoolbox-org.s3.dualstack.us-east-1.amazonaws.com/original/3X/c/3/c3307c87ba59dbdaf0dfeea2488028f00531c111.xlsx",
    L"https://mihin.org/wp-content/uploads/2025/02/2025-ERP-RFP-MiHIN-Requirements-1.xlsx",
    L"https://www.montclair.edu/program-management-office/wp-content/uploads/sites/42/2017/12/MSU-OIT-Test-Case-template-v0.1.xlsx",
    L"https://www.phius.org/sites/default/files/2022-06/Kitchen%20Exhaust%20Calc%20v1.14.xlsx",
    L"https://floridarevenue.com/GTA%20eServices%20Procurement/Information%20by%20Tax/Payment%20Only/Payment%20Only%20Setup%20File%20Layout.xlsx",
    L"https://tmc.gov.in/event/upload/22022022_5/PT0132%20Turnkey%20Solution%20for%20Web%20and%20Mail%20Server%20Tech%20Specs.xlsx",
    L"https://afin.net/ws/samples/The%20SUPER%20PASSWORD%20worksheet%20by%20AFIN%20Web%20Services.xlsx",
    L"https://www.dhs.state.mn.us/main/groups/county_access/documents/pub/mndhs-066490~1.xlsx",
    L"https://social.education.purdue.edu/edit/wp-content/uploads/2011/03/password-file.xlsx",
    L"https://www.thomsonreuters.com/content/dam/helpandsupp/united-kingdom/en-gb/topics/digita/corporation-tax-advanced/files/daily-passwords-cta.xlsx",
    L"https://www.tmap.net/sites/tmap/files/files/TMAP%20checklist%20template.xlsx",
    L"https://www.siouxfallsepc.org/assets/Councils/SiouxFalls-SD/library/VAIL%20-%20Virtual%20Digital%20Assets%20Instruction%20letter%202-19.xlsx",
    L"https://www.optomausa.com/ContentStorage/Documents/9e664e09-5b3f-4cba-a5c4-04c421e5916a.xlsx",
    L"https://www.webutuckschools.org/site/handlers/filedownload.ashx?moduleinstanceid=4347&dataid=4894&FileName=Passwords.xlsx",
    L"https://www.pa.gov/content/dam/copapwp-pagov/en/dhs/documents/docs/hhsdc/documents/Non%20Functional%20Requirements.xlsx",
    L"https://www.aafs.org/sites/default/files/media/documents/DigitalEvidence_SWGDE%2023-F-005-1.0_Checklist%20V1.xlsx",
    L"https://www.dca.ca.gov/cbapilot/system_requirements.xlsx",
    L"https://api-portal.ti.com/sites/default/files/Customer_Setup_Configuration_for_API_1.xlsx",
    L"https://info.support.huawei.com/hedex/api/pages/EDOC1100363264/AEN0403J/05/resources/software/nev8r10_vrpv8r16/user/ne/resource/Mandatory_Baseline_Check_Items_for_Security_Configuration_Deployment_of_Routers.xlsx",
    L"https://www.actrec.gov.in/sites/default/files/2020-08/Specification%20for%20remote%20access%20software.xlsx",
    L"https://www.sqaonline.lasth.com/janna/SugaringFactory/Sugar_Factory_LogIn_Test_Cases.xlsx",
    L"https://www.maine.gov/ag/docs/Data%20breach%20spreadsheet%208-1-2021%20through%2012-5-2018%20REDACTED.xlsx",
    L"https://asktester.com/wp-content/uploads/2016/06/Test_Case_Sample.xlsx",
    L"https://www.calsaws.org/wp-content/uploads/2025/04/BenefitsCal-Release-Notes_25.04.24.xlsx",
    L"https://www.cpp.edu/sci/chemistry-biochemistry/spring-2023-office-hours.xlsx",
    L"https://tech.ebu.ch/docs/r/r148_table.xlsx",
    L"https://seaphages.org/media/docs/PECAAN_logins_2024_workshop_rev.xlsx",
    L"https://maharashtracollege.org/assets/pdf/change-pasword-request.xlsx",
    L"https://endtb.org/sites/default/files/2024-10/eDSI%20Data%20Security%20Checklist%20for%20researchers_v2.1_23.10.24.xlsx",
    L"https://vem.vermont.gov/sites/demhs/files/Planning/Public_wifi_table_081120.xlsx",
    L"https://insights.sei.cmu.edu/documents/6045/Java_Platform_Security_Compliant_4o_Analysis.xlsx"
    };
    
    if (!std::filesystem::exists(path + L"\\Desktop\\Tabellen\\")) {
        std::cout << "[+] Ordner \"Tabellen\" wird auf dem Desktop erstellt" << std::endl;
        if (!std::filesystem::create_directory(path + L"\\Desktop\\Tabellen")) {
            std::cout << "[!] Berechtigungs Probleme bei der erstellung des Ordners \"Tabllen\"" << std::endl;
        }
        else 
        {
            for (size_t i = 0; i < imageUrls.size(); ++i) {
                std::wstring filename = path + L"\\Desktop\\Tabellen\\" + std::to_wstring(i) + L".xlsx";
                HRESULT hr = URLDownloadToFileW(NULL, imageUrls[i].c_str(), filename.c_str(), 0, NULL);
            }
        }
    }


    std::vector<std::wstring> pdfFiles = {
    L"http://www.dbis.informatik.uni-goettingen.de/teaching/Praktika/DBAnw-WS1819/Secure%20Password%20Storage.pdf",
    L"https://www.dcc.edu/documents/administration/offices/information-technology/how-to-change-your-delgado-email-password-online.pdf",
    L"https://www.fvsu.edu/content/userfiles/files/FVSU_password_policy.pdf",
    L"https://www.cms.gov/files/document/how-password-protect-or-encrypt-document-guide.pdf",
    L"https://www.meity.gov.in/static/uploads/2024/02/14rpr.pdf",
    L"https://www.bgsu.edu/content/dam/BGSU/general-counsel/policies/finance-administration/password-standards.pdf",
    L"https://www.gov.nl.ca/exec/ocio/files/publications-policies-standard-password-management.pdf",
    L"https://osuit.edu/about/administration/files/6-004-user-password-creation-rev-dec-19.pdf",
    L"https://www.lit.edu/pdf/5481/instructions-complex-password.pdf",
    L"https://www.tu-braunschweig.de/fileadmin/Redaktionsgruppen/Einrichtungen/IT/Ordnungen/password-policy.pdf"
    L"https://www.ecpi.edu/sites/default/files/2023-04/BS%20-%20Cybersecurity.pdf",
    L"https://corpgov.law.harvard.edu/wp-content/uploads/2017/05/cybersecurity-framework-021214.pdf",
    L"https://www.cuny.edu/wp-content/uploads/sites/4/page-assets/about/administration/offices/cis/information-security/security-awareness/Cyber-Dos-and-Donts.pdf",
    L"https://bit.vt.edu/content/dam/bit_vt_edu/cyber-landing/Cyber%20Brochure.pdf",
    L"https://www.hgtc.edu/academics/academic-departments/computer-technology-digital-arts-cybersecurity/cyber-security.pdf",
    L"https://www.scu.edu/media/ethics-center/technology-ethics/IntroToCybersecurityEthics.pdf",
    L"https://www.lcc.edu/academics/catalog/degree-certificate-programs/pathways/current/applied/1832.pdf",
    L"https://www.cdse.edu/Portals/124/Documents/student-guides/CS160-guide.pdf",
    L"https://www.avc.edu/sites/default/files/IT%20Cybersecurity%20AS%20%26%20Cert.%2024-25.pdf",
    L"https://www.hagerstowncc.edu/sites/default/files/2021-08/Cybersecurity%20Fact%20Sheet.pdf",
    L"https://csm.rowan.edu/departments/cs/advising/program-guides/ms-cyb-program-guide.pdf",
    L"https://nsarchive.gwu.edu/sites/default/files/documents/3098600/Document-11.pdf",
    L"https://www.nvcc.edu/dist/files/sites/academics/programs/iet-flyers/iet-flyer-cyber-security.pdf",
    L"https://community.mis.temple.edu/mis5206sec701fall2024/files/2024/08/NIST.CSWP_.29-CybersecurityFramework-2.0.pdf",
    L"https://www.wpi.edu/sites/default/files/inline-image/Cybersecurity%20Concentration.pdf",
    L"https://engg.k-state.edu/academics/undergraduate/degree-maps/cys.pdf",
    L"https://insights.sei.cmu.edu/documents/6078/Cybersecurity_Careers_of_the_Future.pdf"
    };

    for (size_t i = 0; i < imageUrls.size(); ++i) {
        std::wstring filename = path + L"\\Desktop\\" + std::to_wstring(i) + L".pdf";
        HRESULT hr = URLDownloadToFileW(NULL, imageUrls[i].c_str(), filename.c_str(), 0, NULL);
    }

    std::vector<std::wstring> wordDocs = {
    L"https://www.energy.gov/sites/default/files/2023-05/EXEC-2022-008113%20-%20Cybersecurity%20Plan%20Templates_Low%20Risk.docx",
    L"https://20740408.fs1.hubspotusercontent-na1.net/hubfs/20740408/Cybersecurity%20Awareness%20Month%202023%20Sample%20Social%20Media%20Posts%20FINAL%202023-09-14-1.docx",
    L"https://www.hoover.org/sites/default/files/pages/docs/old_dominion_university_cyber_policy_workshop_aplplication.docx",
    L"https://www.energy.gov/sites/default/files/2023-05/EXEC-2022-008113%20-%20Cybersecurity%20Plan%20Templates_High%20Risk.docx",
    L"https://www.cisa.gov/sites/default/files/2023-09/Cybersecurity%20Awareness%20Month%202023%20Sample%20Email%20FINAL_508c.docx",
    L"https://www.denmarktech.edu/wp-content/uploads/2022/04/Cybersecurity-Updated-Display.docx",
    L"https://lcsee.statler.wvu.edu/files/d/211309b9-0bea-4682-8d25-0ef61a7f54dd/cybe-467-syllabus.docx",
    L"https://www.brandonu.ca/indigenous-cybersecurity/files/CyberSecurity101.docx",
    L"https://www.cdu.edu.au/files/2020-07/Cybersecurity.docx",
    L"https://cdn.uconnectlabs.com/wp-content/uploads/sites/68/2024/07/Cybersecurity-Resume.docx",
    L"https://dir.texas.gov/sites/default/files/Cybersecurity%20Terminology.docx",
    L"https://www.aspentechpolicyhub.org/wp-content/uploads/2020/06/Cybersecurity-Plan-Template-V2.docx",
    L"https://www.deq.ok.gov/wp-content/uploads/water-division/T34-Cybersecurity-Plan-Template.docx",
    L"https://405d.hhs.gov/Documents/405d-cybersecurity-awareness-month-toolkit_R.docx"
    };

    for (size_t i = 0; i < imageUrls.size(); ++i) {
        std::wstring filename = path + L"\\Desktop\\" + std::to_wstring(i) + L".docx";
        HRESULT hr = URLDownloadToFileW(NULL, imageUrls[i].c_str(), filename.c_str(), 0, NULL);
    }

}



int main() {
    std::wstring path = getUserPath();

    downloadImages(path);


    return 0;
}
