/*
 * Modification History
 *
 * 2006-August-11   Jason Rohrer
 * Created.
 */



#ifndef GENETICS_INCLUDED
#define GENETICS_INCLUDED




/**
 * Generic genetics base class.
 *
 * @author Jason Rohrer
 */
class Genetics {

    public:

        
        virtual ~Genetics();


        
        /**
         * Gets the number of genes.
         */
        int getNumGenes();



        /**
         * Gets the length of a gene.
         *
         * @param inIndex the index of the gene in [0, getNumGenes() - 1].
         *
         * @return the length of the gene.
         */
        int getGeneLength( int inIndex );

        
        
        /**
         * Gets a gene.
         *
         * @param inIndex a value in [0, getNumGenes() - 1]
         *
         * @return an array of values in [0,1].
         *   The array has length getGeneLength( inIndex ).
         *   The array must be destroyed by caller.
         */
        double *getGene( int inIndex );


        
        /**
         * Print the genetics to standard out.
         */
        void printGenetics();


        
        /**
         * Gets wether two genetics have equal gene values.
         *
         * @param inGenetics the other genetics to test.
         *   Destroyed by caller.
         *
         * @return true if equal.
         */         
        char equals( Genetics *inGenetics );


        
    protected:

        
        /**
         * Constructs random genetics.
         *
         * @param inNumGenes the number of genes.
         * @param inGeneLengths the length of each gene.  Destroyed by caller.
         */
        Genetics( int inNumGenes, int *inGeneLengths );


        
        /**
         * Constructs genetics by crossing two parent genetics.
         *
         * Parent genetics must be of the same type (same number of genes,
         *   and same length for each pair of corresponding genes).
         *
         * @param inParentA, inParentB the parent genetics.
         *   Destroyed by caller.
         */         
        Genetics( Genetics *inParentA,
                  Genetics *inParentB );

        

        /**
         * Copies another genetics.
         *
         * @param inGeneticsToCopy the genetics to copy.  Destroyed by caller.
         */
        Genetics( Genetics *inGeneticsToCopy );



        /**
         * Map a value in the range [0,1] to a different range.
         *
         * @param inValue the value in [0,1] to map.
         * @param inRangeStart, inRangeEnd the range to map to.
         *
         * @return the mapped value.
         */
        static double mapValueToRange( double inValue, double inRangeStart,
                                       double inRangeEnd );


        
        int mNumGenes;
        int *mGeneLengths;
        double **mGenes;
        
    };



#endif
