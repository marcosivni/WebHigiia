**WARNING: Wia-Spine is NOT clinical software. It was designed for education and demonstration purposes ONLY!**

# Wia-Spine Modeling Guide

Wia-Spine relies on a set of reserved tables and attributes for mapping dataset entries in a _framework_-like environment. Therefore, those tables must be created once in the server-side of the CBMIR application ([SIREN][siren]) before you start any Wia-Spine client. 

A simple telnet connection (`telnet <siren-ip> <siren-port>`) allows you to feed the script to the server-side. The default creation script is available [here][higiiaddl], and the relational representation of the tables are as follows:


- Table Login: For users and passwords (will be deprecated as soon as QSsl gets supported by Emscripten).
- Table Pool: Associates query images with users.
- Scope: Defines the attributes of interest targeted by the CBMIR application (including labeling and visual mining).
- Caption: Provides a textual explanation for the Scope attributes.

**NOTE:** Both `Scope` and `Pool` Tables can be linked with your underlying DBMS data dictionary with foreign key constraints to ensure full consistency of the model (you still can use the client without that, though). Such constraints are not represented here for simplification purposes.

## Setting up Wia-Spine - The HC_LVERT example


In this example, we will configure Wia-Spine to query vertebral bodies from the [HC_LVERT dataset][dataset], which can be straightforwardly translated into a relational representation.

> Attributes {`Id`, `IdStudy`, `Filename`, `Patient_Name`, `Image_Type`, `Image_Class`, `Mask`} are **reserved** and **mandatory** for any dataset modeled after Wia-Spine. Besides, at least one PARTICULATE attribute is expected to be found in the dataset-based table. Wia-Spine can't issue queries over datasets without those attributes.

This table representation can be created on the SIREN server by using extended SQL (through a simple telnet connection), as follows:

```sql
CREATE TABLE Spine40D (
    Id INTEGER,
    IdStudy INTEGER,
    Filename VARCHAR(255),
    Patient_Name VARCHAR(300),
    ReliefF PARTICULATE(40),
    Mask BLOB,
    Image_Type INTEGER,
    Image_Class INTEGER,
    Patient_Sex INTEGER,
    Patient_Age INTEGER,
    Menopause_Age INTEGER,
    Vertebrae INTEGER,
    DEXA FLOAT,
    BMD FLOAT,
    BMI FLOAT,
    Diagnosis INTEGER,
    PRIMARY KEY (Id),
    METRIC (ReliefF) USING (L2, L1, CANBERRA)
) ;

```

While the syntax is standard SQL for the most part, the constraint `METRIC` defines the distance functions associated with the `PARTICULATE` (40-dimensional) `ReliefF` attribute: `L1, L2, CANBERRA`. Those metrics must be previously created by the [Wia-Spine instantiation script][higiiaddl].

Attributes {`Diagnosis`, `Vertebrae`} are optional values that we can set Wia-Spine to focus on (labeling, etc.). For instance, we can set Wia-Spine to keep `Diagnosis` as a **search scope** for the `Spine40D` table with a SQL insert into the Wia-Spine  internal table.

```sql
INSERT INTO Scope (tableName,attributeName) VALUES ('Spine40D', 'Image_Class');
INSERT INTO Scope (tableName,attributeName) VALUES ('Spine40D', 'Patient_Age');
INSERT INTO Scope (tableName,attributeName) VALUES ('Spine40D', 'Menopause_Age');
INSERT INTO Scope (tableName,attributeName) VALUES ('Spine40D', 'DEXA');
INSERT INTO Scope (tableName,attributeName) VALUES ('Spine40D', 'BMD');
INSERT INTO Scope (tableName,attributeName) VALUES ('Spine40D', 'BMI');
INSERT INTO Scope (tableName,attributeName) VALUES ('Spine40D', 'Diagnosis');
INSERT INTO Scope (tableName,attributeName) VALUES ('Spine40D', 'Vertebrae');
```

Additionally, we can set Wia-Spine to provide proper subtitling for the scope attribute by populating its internal Caption table with inserts regarding the `Diagnosis` attribute.

```sql
-- Don't use comma in the captions!
INSERT INTO Caption (tableN,attributeN,valueN,caption) VALUES ('Spine40D', 'Diagnosis', '0', 'Normal');
INSERT INTO Caption (tableN,attributeN,valueN,caption) VALUES ('Spine40D', 'Diagnosis', '1', 'Osteopenia');
INSERT INTO Caption (tableN,attributeN,valueN,caption) VALUES ('Spine40D', 'Diagnosis', '2', 'Osteoporosis');
INSERT INTO Caption (tableN,attributeN,valueN,caption) VALUES ('Spine40D', 'Image_Class', '0', 'Non-Fracture');
INSERT INTO Caption (tableN,attributeN,valueN,caption) VALUES ('Spine40D', 'Image_Class', '1', 'Fracture');
INSERT INTO Caption (tableN,attributeN,valueN,caption) VALUES ('Spine40D', 'Vertebrae', '1', 'L1');
INSERT INTO Caption (tableN,attributeN,valueN,caption) VALUES ('Spine40D', 'Vertebrae', '2', 'L2');
INSERT INTO Caption (tableN,attributeN,valueN,caption) VALUES ('Spine40D', 'Vertebrae', '3', 'L3');
INSERT INTO Caption (tableN,attributeN,valueN,caption) VALUES ('Spine40D', 'Vertebrae', '4', 'L4');
INSERT INTO Caption (tableN,attributeN,valueN,caption) VALUES ('Spine40D', 'Vertebrae', '5', 'L5');
```


After the table creation and scope definition, we can insert dataset elements with extended SQL commands. We have computed the multidimensional features for each dataset entry outside Wia-Spine (by PyRadiomics and other data pre-processors) and then wrapped it up with an insertion script.

Assuming the `ReliefF` features have already been extracted in our example, the data insertion follows extended SQL commands such as:

```sql
INSERT INTO Spine40D (Id,IdStudy, Filename, Patient_Name, ReliefF, Mask, Image_Type, Image_Class, Patient_Sex, Patient_Age, Menopause_Age, Vertebrae, DEXA, BMD, BMI, Diagnosis) VALUES (1,1,'01-T2-S06-L1.jpg','P1', {0.009713949316884,0.074973966832704,0.680273758020002,0.156443614563466,0.042526647775017,0.314249351290943,0.100713243221951,0.216894670788828,0.001292818539848,0.554167383668965,0.2778730703259,0.520987004809167,0.193962176621348,0.372828513630304,0.001699239723309,0.398522772278004,0.095498350574363,0.352105072910631,0.84406837070081,0.307239985299522,0.516942762114049,0.670903953984431,0.516942762114088,0.260539308773263,0.048611107865198,0.249330316794328,0.00062471212486,0.217391304347827,0.000862964434203,0.967627457316227,0.740411449900011,0.090722530456707,0.362671629198511,0.197396541643071,0.629849820468763,0.991086233139921,0.958553460345293,0.396058903716852,0.069474528527306,0.770953764699405}, '',4,1,0,84,48,1,-2.1,0.812,28,1);
INSERT INTO Spine40D (Id,IdStudy, Filename, Patient_Name, ReliefF, Mask, Image_Type, Image_Class, Patient_Sex, Patient_Age, Menopause_Age, Vertebrae, DEXA, BMD, BMI, Diagnosis) VALUES (2,1,'01-T2-S06-L2.jpg','P1', {0.008688694608824,0.186763875633165,0.635781480782126,0.238913985206281,0.136505358319345,0.353971040427553,0.140797831689491,0.162885898911177,0.011015032172543,0.400610956462446,0.277249337283643,0.398353358291706,0.143738093791407,0.508140767003521,0.001970892127256,0.568420184833824,0.117877733981216,0.2442521692859,0.838812813045267,0.310915104740904,0.453291941293433,0.68055021072737,0.453291941293467,0.301481200151918,0.156154012149728,0.736665366423378,0.000215325936445,0.087942796087778,0.000269231969698,0.976694403635815,0.726374544799365,0.064137983261293,0.326548314160083,0.203950999646399,0.620810794578008,0.989514039045221,0.962724959283469,0.267366437566219,0.056529233466051,0.780970828504286}, '',4,1,0,84,48,2,-2.1,0.812,28,1);
INSERT INTO Spine40D (Id,IdStudy, Filename, Patient_Name, ReliefF, Mask, Image_Type, Image_Class, Patient_Sex, Patient_Age, Menopause_Age, Vertebrae, DEXA, BMD, BMI, Diagnosis) VALUES (3,1,'01-T2-S06-L3.jpg','P1', {0.01114994550685,0.062376381821042,0.546919778130328,0.431446892939364,0.018254537579822,0.268993261646456,0.131373397660193,0.104406571309799,0.006004518825378,0.467525856911979,0.276545122558512,0.481530599508231,0.090570016950327,0.614696997974585,0.000192972324797,0.446494437652573,0.084468827266352,0.345462233500023,0.834105085403429,0.307239985299522,0.427912034552118,0.665266134506408,0.427912034552138,0.301481200151918,0.090260445430286,0.787714203651429,8.66811341499311E-05,0.117139777218828,0.000264230477796,0.973860597522915,0.727826342734331,0.01278473402202,0.318941898206536,0.145424720565202,0.609221126050444,0.990266278465397,0.980095349190646,0.104338038484075,0.061198040980534,0.78904577069892}, '',4,1,0,84,48,3,-2.1,0.812,28,1);
INSERT INTO Spine40D (Id,IdStudy, Filename, Patient_Name, ReliefF, Mask, Image_Type, Image_Class, Patient_Sex, Patient_Age, Menopause_Age, Vertebrae, DEXA, BMD, BMI, Diagnosis) VALUES (4,1,'01-T2-S06-L4.jpg','P1', {0.007789705624039,0.116537936065525,0.580587518240591,0.410187372432755,0.032857179939616,0.196104902545015,0.123991339822701,0.065584706261415,0,0.368592896688756,0.27958833619211,0.406196232407726,0.101977574761315,0.513194908553376,0,0.438797275014301,0.076446240901175,0.306246532116383,0.826373092892777,0.312293274531423,0.402615637069245,0.637518142553869,0.40261563706922,0.30307633877706,0.205361475228159,0.630776945450934,0,0.217391304347827,0.000203483393532,0.973665282673256,0.727227475885104,0.036206761872665,0.319451752517882,0.188363392033958,0.616555883955693,0.991706634604769,0.985792089285555,0.047982206188201,0.059972989404536,0.779248067716815}, '',4,1,0,84,48,4,-2.1,0.812,28,1);
```

Optionally, you can provide masks and overlays to the vertebral bodies after data insertion (See examples below).

```sql
UPDATE Spine40D SET Mask='AAoVYHja7dw9TsNAGATQs9nIBQWFT8P1QYgI8keMvZ89q7wnpUI0K41m1ji8D6/D8Da8DAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAPyYp9EhQLrPnM5fH0cBuU45HXUrhG/fs48jgdhOPc+sU4HIe+rVndUOhsT9e+veCqTt3zt3V4cDmfv3agc7Hsjcv+6sENup071dbAdDUqfOf/1Mt0KAB/dR70VAfKde7WDPmODATl2wbXUrHGtx9nQrxHfq763sGRMc06nT/7NtB8O+1vSjrMIR+3dtF3/vYPdX2KVT59UZv/w4T6jr1HHT7/q/EbCDrfmazv93BJDWqbcy60ShrFOndpm3fyG7Uz1TgjrN8uXdJeigU21fKO9U2xeeo1NtX+igU21fKNWoC21fqN++c0jegdoutH0hf/uu/u4csN/21anQxfZt+p4/ULR9h+3fnQPKt++pn4Hk7XvKvTOFqk5tuqUdKtR0amrugbpOtX+hvcbZ0qlQuH+nzNwDl1lt+H0aRwrlWd3WsToV9s3qup7VqbCDzXn1dxo4vmMXZt3xQUDPPs44kJ5Xz5QgbRPbv9BJXu1f6COv9i90cne1fyHb7Tur/Qu5WZ3tX+jqzuqdQujhzuqdQuhiB+tU6CarQD93VgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAI8AFJ+qeZ' WHERE Filename = '01-T1-S07-L1.jpg';
UPDATE Spine40D SET Mask='AAoVYHja7d3LasMwEAXQb5OKF1104a/J75fS0lLHqR8dOSN0DmQTQhaC4d5xbHIrr6W8lZcCAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHDNPtcyOAXL7mNOvF5DVz5zKVkjefX+9nAlkzVQ9GDrYU9e6sJOBnJl614edDqTK1GntPdkKmazvpa4xQb7++3iGXWOCRP13Y4+VrZAhU+tmP7a3QoJMnfZ8RrbCM+3dQ+2tkDxT7a2QIlPno59/kK8yF1o5kY939/XXP+cXCOq/Z7PYrMKFmVr/N+fLa8P2WGiTqcFz5bdXiBffVfVfkKkwbqbGzpVMBZkKMjVu9mUqxIrOVM/dQD+Z6lwhPlNr4u8DvjNQpoJMBdJloOdXQaaCTA3MVKcK4UIz0H0PIFNh8Eydk849IFNBpgIyFUZTZSoMl6meeYOWmZq1SwPt9lQgnkyFMTPVngptMjVutmQqtNHgmpJDhUb9N25WXVOCtrMak4X6LzSf1bX/WdR/oYNZPT6z+i+0VDf/a3ze/T1OE660Nq/6L/Qys/ovdNOP9V/Ia0+26r+Qrgvrv9BHF9Z/oZNs1X+hj71V/4U+ZnVavudoIOfOuujFDgaSzurnfPofVcjr3H3CwHNn1XlATzsrAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAJd6B7KzqLw=' WHERE Filename = '01-T1-S07-L2.jpg';
```
> Mask attribures are optional. They are compacted as base64 strings and inserted as BLOBs into the relational DBMS. If you want to understand how they are generated, please check this [GitHub entry](www).

> **NOTE:** Remember copying the image files to the `fs` directory of the [Websocketfy-Server](https://github.com/marcosivni/websocketfy#generating-the-binary). 

> After the insertion of dataset elements, Wia-Spine can query images by content (following the multidimensional features and the user-defined metrics). Nevertheless, we must assign the query images (potentially undiagnosed entries) to the users that can access them.


## Prepare for querying

While queries can be issued to the server counterpart (SIREN) through extended SQL, Wia-Spine requires the loading of (potentially undiagnosed) query images into a proper 'query pool' as well as granting users the permission to access them.

The set of query images must be loaded in a separated table named with the prefix `U_`. In the HC_LVERT example, this table is `U_Spine40D` and contains all reserved and mandatory attributes of table `Spine40D` plus the `PARTICULATE` attributes employed for mapping the space of features. Therefore, the SQL representation of table `Spine40D` is as follows.


```sql
CREATE TABLE U_Spine40D (
    Id INTEGER,
    IdStudy INTEGER,
    Filename VARCHAR(255),
    Patient_Name VARCHAR(300),
    ReliefF PARTICULATE(40),
    Mask BLOB,
    Vertebrae INTEGER,
    url VARCHAR(300),
    PRIMARY KEY (Id),
    METRIC (ReliefF) USING (L2, L1, CANBERRA)
); 
```

> Attribute {`url`} is **reserved** and **mandatory**. It connects the query image with other HIS systems with link sharing support, such as external PACS-Viewers.

After the table creation, we can populate it with query images (See the query image below).

```sql
INSERT INTO U_Spine40D (Id,IdStudy, Filename, Patient_Name, ReliefF, Mask, Vertebrae, url) VALUES (2,65,'65-T2-S07-T12.jpg','P65', {0.0309533956983222,0.144272450328807,0.382620356373207,0.746676169567615,0.015633247865007,0.252444280359835,0.189752075572283,0.112301107274558,0.0895921939187928,0.521363135871077,0.245787508828574,0.650286562592961,0.108205798459102,0.552708324927532,0.139062989936694,0.303013371141471,0.121381616197598,0.469317213009693,0.760263797429665,0.0545755237045204,0.708806293874334,0.442978673654269,0.708806293874358,0.143562476262818,0.500020030214956,0.314619830665876,0.0318751422158816,0.608695652173913,0.00254641599518563,0.998897733521641,0.679758938800847,1.06467951617444,0.520804420090917,0.595965229130128,0.891782310243978,0.989056216714624,0.764498408056819,0.389051319785572,0.00931587401441205,0.574471967137194}, '', 12, '');

UPDATE U_Spine40D SET Mask='AAhVYHja7d1BasMwFATQs8nFiy51it7/BikEmjq2G8kag0Pfo11kk8WHQfNll36Vz/L981EAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAYGWeSp2NAbKZkivIZ+r+C2QzJVcw6pEjuYKMRYbkCjL9b6cPAkf73/6eBQz2v41cVSOCsf63lSvPsGCs/23mSrZgqP8tMuc5FkT631Puqj0L2jPV2OtkC1p3qqkzg1vZ0g+hbadq27PcZ0DfTtWdLVPl3/e/8f74kyVnFYT3IHsVlGhf0/9gvP+tzz2ZQv+bZArCmarJ73JPgZ0qlgOZgvBOJVNQonfgMgXhncozKpApOEH6nsJEkSmZgij3FHDZTBWZguz7r+4pIL0H2alApuCUTHlHCXJkCq6bKfcUUPL36TIF/uYXTuh/V/wueOv+d8X9DN67/3mfAsL9L5QrmYJfZ1UgW+4pYNnbhv9nh50KwrmSKfg7W3YqOGPH6u5/xgcvc1V7MqX/QTBXnlNB29nTvmPJFLSfPy07ludUcCxX1U4F+R1r3sueOcHxHWt9ZtmpINAF56fMGRAEu6D+B7kz6/EZCJ1ZxZ06nNIFjQPCuTILyO5XJgExuh8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAfW51z+ED' WHERE Id = 2;
```

Next, we must associate the query image with a user through an `INSERT INTO` statement, as follows.

```sql
-- If the user is not defined then create one (this feature will be replaced by QSsl in near future)
INSERT INTO Login VALUES (1, 'UserNick', 'UserPass');
-- Insert a pool entry for the user and the query image
INSERT INTO Pool VALUES (1, 2, 'Spine40D');
```

Wia-Spine is now ready to query the HC_LVERT dataset.

## Notes

- Wia-Spine is NOT clinical software. **It is built for education and demonstration purposes ONLY!**
- Scripts of the example in this guide are available at dir [/model](https://github.com/marcosivni/wia-spine/blob/main/model)
- Images of the example are available at dir [/CBMS_Supplementary_Files](https://github.com/marcosivni/wia-spine/blob/main/CBMS_Supplementary_Files)
- _(C) THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OF THIS SOFTWARE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE._

[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)

   [siren]: <https://github.com/marcosivni/siren>
   [higiiaddl]: <https://github.com/marcosivni/webhigiia/blob/main/model/Higiia_DDL.sql>
   [mammo]: <https://github.com/marcosivni/webhigiia/tree/main/model/example/data/mammo>
   [oq]: <https://github.com/marcosivni/webhigiia/tree/main/model/example/data/mammo/query_example_2.krl>
   [hetland]: <https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.216.5538&rep=rep1&type=pdf>
   [drosou]: <https://www.cs.drexel.edu/~julia/documents/big.2016.0054.pdf>
   [jasbick]: <https://link.springer.com/chapter/10.1007/978-3-030-60936-8_11>
   [santos]: <https://ieeexplore.ieee.org/abstract/document/6881893>
   [kundaha]: <https://github.com/marcosivni/kundaha>
   [brid]: <https://www.researchgate.net/profile/Lucio-Dutra-Santos/publication/262253340_Parameter-free_and_domain-independent_similarity_search_with_diversity/links/5ca4aea4299bf1b86d61d045/Parameter-free-and-domain-independent-similarity-search-with-diversity.pdf>
   [agg]: <https://eprints.ukh.ac.id/id/eprint/186/1/2015_Book_DataMining.pdf>
   [dataset]: <https://github.com/marcosivni/wia-spine/blob/main/CBMS_Supplementary_Files/Dataset.md>
   [birads]: <https://www.acr.org/Clinical-Resources/Reporting-and-Data-Systems/Bi-Rads>
   
